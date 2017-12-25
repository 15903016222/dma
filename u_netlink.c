#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <linux/netlink.h>
#include <linux/socket.h>

#define NETLINK_TEST    (25)
#define TEST_PID        (100)

#define DMA_START_ADDR           0x2f000000     // 496 -- 512M
#define DMA_DATA_LENGTH          0x01000000     // 16M

#define REGION_SIZE              0x00040000
#define CONFIG_START_ADDR_OFFSET 0x00100000
#define SCAN_DATA_MARK_OFFSET    0x00200000
#define DATA_SAVE_BLOCK_SIZE_BIT 1024

#define CONFIG_NUM 11

unsigned int data_addr;
unsigned char *scan_mark  ;
int config[CONFIG_NUM];
unsigned int offset_addr[4] = {0};

#define DmaFrameBuffer         config[0]
#define DataDmaCounter         config[1]
#define BufferInUse            config[2]
#define ScanSource             config[3]
#define StoreFrameCount        config[4]
#define EncoderCounterOffset   config[5]
#define StepsPerResolution     config[6]
#define ScanZeroIndexOffset    config[7]
#define MaxStoreIndex          config[8]
#define ScanTimmerCounter      config[9]
#define ScanTimmerCircled      config[10]

/*
 * Save data ro file
 */
void save_data_to_file (int fd)
{
    int EncoderIndex   ;
    int *pEncoderIndex ;
    int nOffset        ;
    off_t offset       ;
    size_t size = 0    ;
    size_t tmp = 0     ;
    size_t res = -1    ;

    size = ((StoreFrameCount / 4) + 1) * DATA_SAVE_BLOCK_SIZE_BIT;
    if (ScanSource) {
        /*** encoder ***/
        nOffset = DataDmaCounter & 0x03;
        pEncoderIndex = &EncoderIndex;
        memcpy((void *)pEncoderIndex, (void*)(nOffset *  REGION_SIZE + EncoderCounterOffset + data_addr) , 4);
        EncoderIndex  = EncoderIndex / StepsPerResolution + ScanZeroIndexOffset;
        if(EncoderIndex > MaxStoreIndex || EncoderIndex < 0) {
            /* out of range , do not dma */
            return ;
        }
        scan_mark[EncoderIndex] = 0xff;
        offset = EncoderIndex * size;
    } else {
        /*** time ***/
        if (ScanTimmerCounter > MaxStoreIndex) {
            /* out of range , reset from head */
            ScanTimmerCounter = 0;
            ScanTimmerCircled++;
        }
        scan_mark[ScanTimmerCounter] = 0xff;
        offset = ScanTimmerCounter * StoreFrameCount * size;    // address to store
    }

    lseek (fd, offset, SEEK_SET);
    size = ((StoreFrameCount / 4) + 1) * DATA_SAVE_BLOCK_SIZE_BIT;
    tmp = size;
    while (tmp) {
        res = write (fd, (void *)offset_addr[DataDmaCounter & 0x03], tmp);
        if (res < 0) {
            perror ("write");
            return ;
        }
        tmp -= res;
    }
    ScanTimmerCounter++;

    return;
}

int netlink_create_socket (void)
{
    //create a socket
    return socket (AF_NETLINK, SOCK_RAW, NETLINK_TEST);
}

int netlink_bind (int sock_fd)
{
    struct sockaddr_nl addr;

    memset (&addr, 0, sizeof (struct sockaddr_nl));
    addr.nl_family = AF_NETLINK;
    addr.nl_pid = TEST_PID;
    addr.nl_groups = 0;

    return bind (sock_fd, (struct sockaddr *)&addr, sizeof (struct sockaddr_nl));
}

int netlink_recv_message (int sock_fd, int *message, int len)
{
    struct nlmsghdr *nlh = NULL;
    struct sockaddr_nl source_addr;
    struct iovec iov;
    struct msghdr msg;

    //create message
    nlh = (struct nlmsghdr *)malloc (NLMSG_SPACE (len));
    if (!nlh) {
        perror ("malloc");
        return -2;
    }
    iov.iov_base = (void *)nlh;
    iov.iov_len = NLMSG_SPACE (len);
    memset (&source_addr, 0, sizeof (struct sockaddr_nl));
    memset (&msg, 0, sizeof (struct msghdr));
    msg.msg_name = (void *)&source_addr;
    msg.msg_namelen = sizeof (struct sockaddr_nl);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    if (recvmsg(sock_fd, &msg, MSG_WAITALL) < 0) {
        printf("recvmsg error!\n");
        return -3;
    }
    memcpy (message, (void *)NLMSG_DATA (nlh), len);

    free (nlh);
    return 0;
}

int main(int argc, char **argv)
{
    int sock_fd = -1;
    int data_fd = -1;
    int file_fd = -1;
    int tmp = 0;
    int len = 0;

    file_fd = open ("data.txt", O_RDWR | O_CREAT);
    if (-1 == file_fd) {
        perror ("open");
        return -1;
    }

    data_fd = open("/dev/mem", O_RDWR | O_NDELAY);
    if (-1 == data_fd) {
        perror ("open");
        close (file_fd);
        return -1;
    }

    data_addr = (unsigned int)mmap (0,  DMA_DATA_LENGTH,
                             PROT_READ | PROT_WRITE, MAP_SHARED, data_fd, DMA_START_ADDR);
    if (NULL == (void *)data_addr) {
        perror ("mmap");
        close (file_fd);
        close (data_fd);
        return -1;
    }
    scan_mark = (unsigned char*)(data_addr + SCAN_DATA_MARK_OFFSET)  ;
    offset_addr[0] = data_addr;
    offset_addr[1] = data_addr + 1 * REGION_SIZE;
    offset_addr[2] = data_addr + 2 * REGION_SIZE;
    offset_addr[3] = data_addr + 3 * REGION_SIZE;

    sock_fd = netlink_create_socket ();
    if (sock_fd == -1) {
        perror ("socket");
        close (file_fd);
        close (data_fd);
        return -1;
    }

    if (netlink_bind(sock_fd) < 0) {
        perror ("bind");
        close (file_fd);
        close (data_fd);
        close (sock_fd);
        return -1;
    }

    int i = 0, count = 0;
    for (i = 0; 1; ++i) {
        if (netlink_recv_message (sock_fd, config, sizeof (config)) == 0) {
            save_data_to_file (file_fd);
		}
	}
    printf ("i[%d] count[%d] \n", i, count);

    munmap ((void *)data_addr, DMA_DATA_LENGTH);
    close (sock_fd);
    close (data_fd) ;
    close (file_fd);
    return 0;
}
