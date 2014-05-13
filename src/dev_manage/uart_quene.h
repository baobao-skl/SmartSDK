#ifndef __UART_QUENE_H__
#define __UART_QUENE_H__

#include "dev.h"

#define UART_QUENE_NUM 20
#define UART_DATA_NUM 6

typedef struct {
	signed char id;
	unsigned char data[UART_DATA_NUM];
}uart_quene_t;

void uart_data_quene_init(void);
void uart_data_queuing(unsigned char *data,unsigned char len);
BOOL uart_check_from_queue(unsigned char *checkdata,unsigned char checklen,unsigned char *getdata, unsigned char getlen);

#endif
