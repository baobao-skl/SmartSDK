#include "uart_quene.h"
#include <unistd.h>

static uart_quene_t uart_data_quene[UART_QUENE_NUM];

void uart_data_quene_init(void)
{
	int i = 0;
	for (i = 0; i < UART_QUENE_NUM; i++) {
		uart_data_quene[i].id = -1;
	}	
}

void uart_data_queuing(unsigned char *data,unsigned char len)
{
	int i = 0, j = 0;
	for (i = 0; i < UART_QUENE_NUM; i++) {
		if (uart_data_quene[i].id == -1)
			break;
	}
	
	if (i < UART_QUENE_NUM) {
		uart_data_quene[i].id = i;
		for(j = 0; j < len; j++)
			uart_data_quene[i].data[j] = *(data + j);
	}else{
		DEBUG_MSG("uart data queue is full\n");
	}	
}

BOOL uart_check(unsigned char *checkdata,unsigned char checklen,unsigned char *getdata,unsigned char getlen)
{
	int i = 0, j = 0,count = 0, k = 0;
	for (i = 0; i < UART_QUENE_NUM; i++) {
		if (uart_data_quene[i].id != -1){
			//handler data..........
			count = 0;
			for(j = 0; j < checklen; j++){
				if(uart_data_quene[i].data[j] == *(checkdata + j)){
					count++;
				}else{
					continue;
				}		
			}
			if(count >= checklen){
				for(k = 0; k < getlen; k++){
					*(getdata + k) = uart_data_quene[i].data[k];
				}
				uart_data_quene[i].id = -1;//clean queue
				return TRUE;
			}
		}
	}
	return FALSE;
}

BOOL uart_check_from_queue(unsigned char *checkdata,unsigned char checklen,unsigned char *getdata,unsigned char getlen)
{
	#define MAX_TRY_TIMES 300
	unsigned char i = 0;
	for(i = 0; i < MAX_TRY_TIMES;i++){
		if(uart_check(checkdata,checklen,getdata,getlen)){
			return TRUE;
		}
		usleep(10000);
	}
	return FALSE;
}
