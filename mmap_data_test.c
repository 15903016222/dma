#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

int main(int argc, char *argv[])
{
    int fd;
	int i;

    unsigned int *data_addr;

    if(argc != 2) {
        printf("Usage: %s <on|off>\n", argv[0]);
        return -1;
    }

    fd = open("/dev/dma_data", O_RDWR);
    if (fd < 0)
        return -1;

    data_addr = (unsigned int *)mmap(0, 0x00020000 * 5, PROT_READ|PROT_WRITE,
                    MAP_SHARED, fd, 0);
	for (i = 0; i < 0x00020000 * 5 / 4; ++i) {
		printf ("%08X ", *(unsigned int *)(data_addr + i));
		if (i % 8 == 7) {
			printf ("\n");
		}
	}

    munmap(data_addr, 0x00020000 * 5);
    close(fd);
    return 0;
}






