#ifndef __UART_COMMON_H__
#define __UART_COMMON_H__

#include "dev.h"

enum control_cmd SWGetCurrentState(const char *mac, const char *name,const unsigned char dev_number);
BOOL ForwardControlToUart(const char *mac,const char *name,DEV_TYPE_INDEX dev_type,const unsigned char dev_cmd,const unsigned char dev_number);
void *uart_thread(void *);

#endif
