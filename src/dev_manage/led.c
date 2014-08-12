#include "led.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

void led_control(led_item item,led_state state)
{
	int fd=open("/dev/led",0);
	if(fd<0){
		return;
	}
	ioctl(fd,state,item);
	close(fd);
}

void led_flash(led_item item,int interval,int times)
{
	char cmd[20] = {0};
	sprintf(cmd,"led_flash %d %d %d&",item,interval,times);
	system(cmd);
}


