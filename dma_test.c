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

    unsigned int *addr;
 
    if(argc < 4) {
        printf("Usage: %s </dev/dma_data|/dev/dma_buffer> <start_addr> <size>\n", argv[0]);
        return -1;
    }

    fd = open(argv[1], O_RDWR);
    if (fd < 0)
        return -1;

	if (!strcmp (argv[1], "/dev/dma_buffer")) {
		addr = (unsigned int *)mmap(0, 0x20000000, PROT_READ|PROT_WRITE,
			            MAP_SHARED, fd, 0);

		for (i = 0; i < (convert (argv[3])) / 4; ++i) {
			printf ("%08X ", *(unsigned int *)(addr + (convert (argv[2]) - 0x30000000) / 4 + i));
			if (i % 8 == 7) {
				printf ("\n");
			}
		}
    munmap(addr, 0x20000000);
	}
	else if (!strcmp (argv[1], "/dev/dma_data")) {
		addr = (unsigned int *)mmap(0, 0x01000000, PROT_READ|PROT_WRITE,
			            MAP_SHARED, fd, 0);

		for (i = 0; i < (convert (argv[3])) / 4; ++i) {
			printf ("%08X ", *(unsigned int *)(addr + (convert (argv[2]) - 0x2f000000) / 4 + i));
			if (i % 8 == 7) {
				printf ("\n");
			}
		}
	}

    close(fd);
    return 0;
}






