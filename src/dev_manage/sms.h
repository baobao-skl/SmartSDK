#ifndef __SMS_H__
#define __SMS_H__

#include "dev.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <error.h>
#include <termios.h>
#include <malloc.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define CENTER_NUMBER "+8613010888500"
#define SMS_DEV_NAME "/dev/ttyUSB0"
#define GSM_LOG_PATH "/var/log/gsm.txt"
#define MAX_BUFFER_SIZE 512

typedef struct{
	int fd;
	BOOL (*init)(void);
	BOOL (*send)(const char *_to,const char *_msg);
}sms_t;

typedef enum{
	CHARSET_8BITS = 0,
	CHARSET_7BITS,
	CHARSET_UCS2
}charset_t;

typedef struct {
	char center_number[20];
	char send_number[20];
	char send_time[20];
	char send_content[MAX_BUFFER_SIZE];
	charset_t charset;
}message_info_t;

typedef struct{
	unsigned char  year;
	unsigned char  month;
	unsigned char  day;
	unsigned char  hour;
	unsigned char  minute;
	unsigned char  second;
}time_info_t;

message_info_t handler_receive_msg(char *src);
BOOL save_gsm_log(message_info_t history_info);
void substring(const char *src,char *dest,int index,int length);

#endif

