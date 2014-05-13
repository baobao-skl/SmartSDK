#ifndef __DEV_CONTROL_H__
#define __DEV_CONTROL_H__

#include "dev.h"

enum control_cmd switch_dev_get_state(const char *ip,const unsigned char dev_number);
BOOL switch_dev_control_func(const char *ip,const unsigned char dev_cmd,const unsigned char dev_number);
BOOL chuanglian_dev_control_func(const char *ip,const unsigned char dev_cmd);
BOOL door_dev_control_func(const char *ip,const unsigned char dev_cmd);

struct dev_cmd_t{
	enum type_index index;
	DEV_TYPE_LIST header;
	unsigned char open;
	unsigned char close;
	unsigned char pause;
	unsigned char toggle;
	unsigned char get_state;
	unsigned char len;
};

struct dev_cmd_t * get_cmd_list(void);
void get_ip_byte(const char *ip_str,unsigned char *ip_byte);


#endif
