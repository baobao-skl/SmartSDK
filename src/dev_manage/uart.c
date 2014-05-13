#include "uart.h"
#include <termios.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "dev.h"
#include <stdio.h>
#include <string.h>

static BOOL ReadFromUart(char *buffer);
static BOOL WriteToUart(unsigned char *buffer,unsigned char len);
static BOOL Uart_Config(void);

uart_t uart = {
	0,
	Uart_Config,
	WriteToUart,
	ReadFromUart
};

static BOOL WriteToUart(unsigned char *buffer,unsigned char len)
{
	if(write(uart.fd,buffer,len) != len){
		DEBUG_MSG("send fail\n");
		return FALSE;
	}
	DEBUG_MSG("send OK\n");
	return TRUE;
}

static BOOL ReadFromUart(char *buffer)
{
	char temp[50] = {0};
	BOOL isOK = FALSE;
	while(read(uart.fd,temp,sizeof(temp))>0)
	{
		if(strchr(temp,'*')!=NULL){//start flag *
			memset(buffer,0,sizeof(buffer));
			strcat(buffer,temp);
		}
		else if(strchr(temp,'#')!=NULL){//end flag #
			strcat(buffer,temp);
			isOK = TRUE;
			break;
		}
		else{
			strcat(buffer,temp);
		}
		memset(temp,0,sizeof(temp));
	}
	return isOK;
}

static BOOL Uart_Config(void)
{
	struct termios options;//|O_NONBLOCK))<0){
	if((uart.fd = open(UART_DEV_NAME,O_RDWR|O_NOCTTY))<0){
		close(uart.fd);
		return FALSE;
	}
	memset(&options, 0, sizeof(options));
	options.c_iflag = IGNPAR;
	options.c_cflag = B9600 | HUPCL | CS8 | CREAD | CLOCAL;
	//options.c_cc[VMIN] = 1;
	if (tcsetattr(uart.fd, TCSANOW, &options) != 0) {
	    return FALSE;
	}
	return TRUE;
}



