#ifndef __UART_H__
#define __UART_H__
#include "dev.h"

#define UART_DEV_NAME "/dev/ttySAC2"

typedef struct{
	int fd;
	BOOL (*init)(void);
	BOOL (*write)(unsigned char *buffer,unsigned char len);
	BOOL (*read)(char *buffer);
}uart_t;

#endif
