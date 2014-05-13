#include "led.h"
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

void led_flash(led_item item,int interval,int times);

int main(int argc,char **argv)
{
	int item = 0,interval = 0,times = 0;
	if(argc != 4)
		return -1;
	item = strtol(argv[1],NULL,16);
	interval = strtol(argv[2],NULL,16);
	times = strtol(argv[3],NULL,16);
	if(item > LED_MAX)
		return -1;
	led_flash(item,interval,times);
	return 0;
}

void led_flash(led_item item,int interval,int times)
{
	int fd = 0 , i = 0;
	fd=open("/dev/led",0);
	if(fd<0){
		return;
	}
	for(i = 0;i < times;i++){
		ioctl(fd,ON,item);
		usleep(20000);
		ioctl(fd,OFF,item);
		if(i!=(times -1))
			usleep(20000);
	}
	close(fd);
}


