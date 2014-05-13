#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/time.h>
#include <errno.h>
#include <string.h>
#include <malloc.h>

int main(int argc,char **argv)
{
	unsigned char value = 0;
    int fd = -1;
	char *buf = malloc(sizeof(char)*12);
    fd = open("/dev/nrf24l01", O_RDWR); //打开nrf24l01为可读写文件
    if(fd < 0)
    {
        perror("Can't open /dev/nrf24l01 \n");
        exit(1);
    }

	value = strtol(argv[1],NULL,10);
	if(value==1)
	{
		strcpy(buf,"560304050607");
	}
	else if(value == 0)
	{
		strcpy(buf,"780304050607");
	}
    if(write(fd, buf, strlen(buf))==0)
	printf("Sending OK\n");
    else
	printf("Sending fail\n");
	free(buf);
    close(fd);
	return 0;
}
