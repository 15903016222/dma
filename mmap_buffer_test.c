#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

unsigned int convert (char *str) {
    unsigned int i,num = 0;

    if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) {
        for(i = 0;str[i] != '\0';i++) {
            if(str[i]>='0' && str[i]<='9')
             num = num*16 + str[i]-'0';
           else if(str[i]<='f' && str[i]>='a')
                num = num*16 + str[i]-'a' + 10; 
            else if(str[i]<='F' && str[i]>='A')
                num = num*16 + str[i]-'A' + 10; 
        }
    }   
    else {
        num = atoi (str);
    }   

    return num;
}

int main(int argc, char *argv[])
{
    int fd;
	int i;

    unsigned int *buff_addr;
 
    if(argc < 3) {
        printf("Usage: %s <start_addr> <length>\n", argv[0]);
        return -1;
    }

    fd = open("/dev/dma_buffer", O_RDWR);
    if (fd < 0)
        return -1;

    buff_addr = (unsigned int *)mmap(0, 0x20000000, PROT_READ|PROT_WRITE,
                    MAP_SHARED, fd, 0);

	for (i = 0; i < (convert (argv[2])) / 4; ++i) {
		printf ("%08X ", *(unsigned int *)(buff_addr + (convert (argv[1]) - 0x30000000) / 4 + i));
		if (i % 8 == 7) {
			printf ("\n");
		}
	}

    munmap(buff_addr, 0x20000000);
    close(fd);
    return 0;
}






