#include "uart.h"
#include "dev_control.h"
#include "dev.h"
#include <stdlib.h>
#include <string.h>
#include "uart_quene.h"

extern uart_t uart;

static struct dev_cmd_t cmd_list[TYPE_INDEX_MAX] = {
	/*type open close toggle get_state*/
	{TYPE_SW_DEV,		DEV_SWITCH,			0x01,0x02,0xff,0x03,0x04,	6},
	{TYPE_CL_DEV,		DEV_CHUANGLIAN,		0x01,0x02,0x03,0xff,0x04,	5},
	{TYPE_DOOR_DEV,		DEV_DOOR,			0x01,0xff,0xff,0xff,0xff,	5},
	{TYPE_KONGTIAO_DEV, 	DEV_HONGWAI_REMOTE,	0x01,0x02,0x03,0xff,0xff,	6},
	{TYPE_TV_DEV, 		DEV_HONGWAI_REMOTE,	0x01,0x02,0x03,0xff,0xff,	6}
};

struct dev_cmd_t * get_cmd_list(void)
{
	return cmd_list;
}

void get_ip_byte(const char *ip_str,unsigned char *ip_byte)
{
	char bytes1[3] = {0};
	char bytes2[3] = {0};

	if(ip_str == NULL){
		*ip_byte = 0xFF;
		*(ip_byte + 1) = 0xFF;
		return;
	}
	
	bytes1[0] = ip_str[0];bytes1[1] = ip_str[1];
	bytes2[0] = ip_str[2];bytes2[1] = ip_str[3];

	*ip_byte = strtol(bytes2,NULL,16);
	*(ip_byte + 1) = strtol(bytes1,NULL,16);
}

enum control_cmd switch_dev_get_state(const char *ip,const unsigned char dev_number)
{
	unsigned char state[6] = {0};
	unsigned char cmd[6] = {0};
	cmd[0] = cmd_list[TYPE_SW_DEV].header;
	get_ip_byte(ip, cmd + 1);
	cmd[3] = cmd_list[TYPE_SW_DEV].get_state;
	cmd[4] = dev_number;
	cmd[5] = 0xFF;
	DEBUG_MSG("%s: %.2X%.2X%.2X%.2X%.2X%.2X\n",__FUNCTION__,cmd[0],cmd[1],cmd[2],cmd[3],cmd[4],cmd[5]);
	if (uart.write(cmd,cmd_list[TYPE_SW_DEV].len)==FALSE) {
		return CMD_NULL;
	}
	if (uart_check_from_queue(cmd,5,state,6)) {
		DEBUG_MSG("%s: %.2X%.2X%.2X%.2X%.2X%.2X\n",__FUNCTION__,state[0],state[1],state[2],state[3],state[4],state[5]);
		if (state[5] == cmd[4]) {
			return (dev_number == 0) ? CMD_SW_CLOSE : CMD_SW_OPEN;
		} else {
			return (dev_number == 0) ? CMD_SW_OPEN : CMD_SW_CLOSE;
		}
	}
	return CMD_NULL;
}

BOOL switch_dev_control_func(const char *ip,const unsigned char dev_cmd,const unsigned char dev_number)
{
	unsigned char cmd[6] = {0};
	unsigned char state[6] = {0};
	cmd[0] = cmd_list[TYPE_SW_DEV].header;
	get_ip_byte(ip, cmd + 1);
	
	if (dev_cmd == CMD_SW_OPEN) {
		cmd[3] = (dev_number == 0) ? cmd_list[TYPE_SW_DEV].close : cmd_list[TYPE_SW_DEV].open;
		cmd[5] = (dev_number == 0) ? 0x20 : 0x10;
	} else if(dev_cmd == CMD_SW_CLOSE) {
		cmd[3] = (dev_number == 0) ? cmd_list[TYPE_SW_DEV].open : cmd_list[TYPE_SW_DEV].close;
		cmd[5] = (dev_number == 0) ? 0x10 : 0x20;
	} else if(dev_cmd == CMD_SW_TOGGLE) {
		cmd[3] = cmd_list[TYPE_SW_DEV].toggle;
		cmd[5] = 0x40;
	} else if(dev_cmd == CMD_SW_GET_STATE) {
		cmd[3] = cmd_list[TYPE_SW_DEV].get_state;
		cmd[5] = 0x0F;
	}
	cmd[4] = (dev_number == 0) ? 1:dev_number;
	DEBUG_MSG("%s: %.2X%.2X%.2X%.2X%.2X%.2X\n",__FUNCTION__,cmd[0],cmd[1],cmd[2],cmd[3],cmd[4],cmd[5]);
	if (uart.write(cmd,cmd_list[TYPE_SW_DEV].len)==FALSE) {
		return FALSE;
	}
	return uart_check_from_queue(cmd,6,state,6);
}



BOOL chuanglian_dev_control_func(const char *ip,const unsigned char dev_cmd)
{
	unsigned char cmd[5] = {0};

	cmd[0] = cmd_list[TYPE_CL_DEV].header;

	get_ip_byte(ip, cmd + 1);
	
	if (dev_cmd == CMD_CL_OPEN) {
		cmd[3] = cmd_list[TYPE_CL_DEV].open;
		cmd[4] = 0x10;
	} else if(dev_cmd == CMD_CL_CLOSE) {
		cmd[3] = cmd_list[TYPE_CL_DEV].close;
		cmd[4] = 0x20;
	} else if(dev_cmd == CMD_CL_PAUSE) {
		cmd[3] = cmd_list[TYPE_CL_DEV].pause;
		cmd[4] = 0x30;
	} else if(dev_cmd == CMD_CL_GET_STATE) {
		cmd[3] = cmd_list[TYPE_CL_DEV].get_state;
		cmd[4] = 0xFF;
	}
	
	DEBUG_MSG("%s: %.2X%.2X%.2X%.2X%.2X\n",__FUNCTION__,cmd[0],cmd[1],cmd[2],cmd[3],cmd[4]);
	return uart.write(cmd,cmd_list[TYPE_CL_DEV].len);
}

BOOL door_dev_control_func(const char *ip,const unsigned char dev_cmd)
{
	unsigned char cmd[6] = {0};
	
	cmd[0] = cmd_list[TYPE_DOOR_DEV].header;
	get_ip_byte(ip, cmd + 1);
	
	if (dev_cmd == CMD_DOOR_OPEN) {
		cmd[3] = cmd_list[TYPE_DOOR_DEV].open;
	} else if(dev_cmd == CMD_DOOR_CLOSE) {
		cmd[3] = cmd_list[TYPE_DOOR_DEV].close;
	} else if(dev_cmd == CMD_DOOR_GET_STATE) {
		cmd[3] = cmd_list[TYPE_DOOR_DEV].get_state;
	}
	
	cmd[4] = 0xFF;
	DEBUG_MSG("%s: %.2X%.2X%.2X%.2X%.2X\n",__FUNCTION__,cmd[0],cmd[1],cmd[2],cmd[3],cmd[4]);
	return uart.write(cmd,cmd_list[TYPE_DOOR_DEV].len);
}

BOOL kongtiao_dev_control_func(const char *ip,const unsigned char dev_cmd,const unsigned char number)
{
	unsigned char cmd[6] = {0};
	//unsigned char state[6] = {0};
	cmd[0] = cmd_list[TYPE_KONGTIAO_DEV].header;
	get_ip_byte(ip, cmd + 1);
	if (dev_cmd == CMD_HONGWAI_ADD) {
		cmd[3] = cmd_list[TYPE_KONGTIAO_DEV].open;
		cmd[5] = 0x10;
	} else if(dev_cmd == CMD_HONGWAI_DEL) {
		cmd[3] = cmd_list[TYPE_KONGTIAO_DEV].close;
		cmd[5] = 0x20;
	} else if(dev_cmd == CMD_HONGWAI_SEND) {
		cmd[3] = cmd_list[TYPE_KONGTIAO_DEV].pause;
		cmd[5]  = 0x30;
	}
	cmd[4] = number;
	DEBUG_MSG("%s: %.2X%.2X%.2X%.2X%.2X%.2X\n",__FUNCTION__,cmd[0],cmd[1],cmd[2],cmd[3],cmd[4],cmd[5]);
	if (uart.write(cmd,cmd_list[TYPE_KONGTIAO_DEV].len)==FALSE)
		return FALSE;
	return TRUE;
	//return uart_check_from_queue(cmd,6,state,6);
}

BOOL tv_dev_control_func(const char *ip,const unsigned char dev_cmd,const unsigned char number)
{
	unsigned char cmd[6] = {0};
	//unsigned char state[6] = {0};
	cmd[0] = cmd_list[TYPE_TV_DEV].header;
	get_ip_byte(ip, cmd + 1);
	if (dev_cmd == CMD_HONGWAI_ADD) {
		cmd[3] = cmd_list[TYPE_TV_DEV].open;
		cmd[5] = 0x10;
	} else if(dev_cmd == CMD_HONGWAI_DEL) {
		cmd[3] = cmd_list[TYPE_TV_DEV].close;
		cmd[5] = 0x20;
	} else if(dev_cmd == CMD_HONGWAI_SEND) {
		cmd[3] = cmd_list[TYPE_TV_DEV].pause;
		cmd[5]  = 0x30;
	}
	cmd[4] = number;
	DEBUG_MSG("%s: %.2X%.2X%.2X%.2X%.2X%.2X\n",__FUNCTION__,cmd[0],cmd[1],cmd[2],cmd[3],cmd[4],cmd[5]);
	if (uart.write(cmd,cmd_list[TYPE_TV_DEV].len)==FALSE)
		return FALSE;
	return TRUE;
}

