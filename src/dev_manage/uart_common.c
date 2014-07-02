#include "uart_common.h"
#include "uart.h"
#include "dev.h"
#include "dev_add.h"
#include "dev_control.h"
#include "dev_db.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "sms_common.h"
#include "wechat.h"
#include "mail_common.h"
#include "net_broadcast.h"
#include "string_common.h"
#include "led.h"
#include "ring.h"
#include "uart_quene.h"

extern uart_t uart;
void SetDefaultAlarmUnicodeName(DEV_TYPE_LIST dev_type,char *unicode_name);
void SetDefaultAlarmName(DEV_TYPE_LIST dev_type,char *name);

struct dev_cmd_t * cmd_list_p = NULL;

enum control_cmd SWGetCurrentState(const char *mac, const char *name,const unsigned char dev_number)
{
	enum control_cmd state = CMD_NULL;
	char ip[10] = {0};
	if(isMacAndNameMatch(mac, name)==FALSE){
		return state;
	}
	if(GetDevIPByMac(mac,ip)==TRUE)
	{
		state = switch_dev_get_state(ip, dev_number);
	}
	return state;
}

/*network forword to uart*/
BOOL ForwardControlToUart(const char *mac,const char *name,DEV_TYPE_INDEX dev_type,const unsigned char dev_cmd,const unsigned char dev_number)
{
	//temp not use . for chuanglian
	if(isMacAndNameMatch(mac, name)==FALSE){
		DEBUG_MSG("mac and name not match\n");
		return FALSE;
	}
	char ip[10] = {0};
	BOOL ret = FALSE;
	if(GetDevIPByMac(mac,ip)==TRUE)
	{
		DEBUG_MSG("mac:%s,ip:%s,name=%s,dev_type = %d,dev_cmd = %d,dev_number=%d\n",mac,ip,name,dev_type,dev_cmd,dev_number);
		switch(dev_type)
		{
			case TYPE_SW_DEV:
				ret = switch_dev_control_func(ip,dev_cmd,dev_number);
				break;
			case TYPE_CL_DEV:
				ret = chuanglian_dev_control_func(ip,dev_cmd);
				break;
			case TYPE_DOOR_DEV:
				ret = door_dev_control_func(ip,dev_cmd);
				break;
			case TYPE_KONGTIAO_DEV:
				ret = kongtiao_dev_control_func(ip,dev_cmd,dev_number);
				break;
			default:
				break;
		}
	}
	else
	{
		DEBUG_MSG("no IP found\n");
	}
	return ret;
}

//this is needed to coordinate
void *uart_thread(void *arg)
{
	char buffer[1024] = {0};
	dev_tt dev;
	char cmd[3] = {0};
	cmd_list_p = get_cmd_list();
	uart_data_quene_init();
	while(TRUE)
	{
		memset(&dev,0,sizeof(&dev));
		if(uart.read(buffer))
		{
			DEBUG_MSG("zigbee receive:%s",buffer);
			buffer[strlen(buffer)-3]='\0';//remove '\n#' at the end
			//first read 3 bytes to decide which command
			char * uart_header = strtok(buffer,",");
			if(uart_header == NULL){
				continue;
			}
			led_flash(LED_FOR_ZIGBEE_NET,1,4);
			int length = strlen(uart_header);
			cmd[0] = uart_header[length-2];
			cmd[1] = uart_header[length-1];
			cmd[2] = '\0';
			DEV_TYPE_LIST uart_cmd_type =(DEV_TYPE_LIST)(strtol(cmd,NULL,16));
			DEBUG_MSG("cmd_type = 0x%x\n",(unsigned int)uart_cmd_type);
			switch(uart_cmd_type)
			{
				case DEV_ADD:
					dev.basic_info.ip= strtok(NULL,",");
					dev.basic_info.mac = strtok(NULL,",");
					if(dev.basic_info.ip == NULL || dev.basic_info.mac == NULL){
						break;
					}
					DEBUG_MSG("add dev:ip = %s,mac = %s\n",dev.basic_info.ip,dev.basic_info.mac);
					AddDevToDynamicTable(dev.basic_info.mac,dev.basic_info.ip);
					break;
				case DEV_SOUND:
				case DEV_YUDI:
				case DEV_YANWU:
				case DEV_HONGWAI:
					dev.basic_info.ip = strtok(NULL,",");
					DEBUG_MSG("ip = %s\n",dev.basic_info.ip);
					if(dev.basic_info.ip == NULL){
						break;
					}
					if(strtok(NULL,",")!=NULL){
						char * state = strtok(NULL,",");
						char name[1024] ={0};
						char unicode_name[1024] = {0};
						alarm_t alarm;
						if(state == NULL){
							break;
						}
						memset(&alarm,0,sizeof(alarm_t));
						DEBUG_MSG("state = %s\n",state);
						if(GetDevNameByIP(dev.basic_info.ip, name)==FALSE)
							SetDefaultAlarmName(uart_cmd_type,name);
						if(GetDevUnicodeNameByIP(dev.basic_info.ip,unicode_name)==FALSE)
							SetDefaultAlarmUnicodeName(uart_cmd_type,unicode_name);
						DEBUG_MSG("alarm id = %d state = %s name = %s!!\n",uart_cmd_type,state,name);
						strcpy(alarm.alarm_name,name);
						strcpy(alarm.alarm_unicode_name,unicode_name);
						if(strstr(state,STR_UART_ALARM_COME)){//alarm
							strcpy(alarm.alarm_value,STR_CHS_SYS_ALARM_COME);
							strcpy(alarm.alarm_unicode_value,STR_UNI_SYS_ALARM_COME);
							send_broadcast(STR_ALARM_SEND_BROADCAST_PRE, ACTION_SYS_ALARM_COME, alarm.alarm_name);
							system_ring(DEV_ALARM_RING_CMD);
						}else if(strstr(state,STR_UART_ALARM_GONE)){//xiaochu
							strcpy(alarm.alarm_value,STR_CHS_SYS_ALRAM_GONE);
							strcpy(alarm.alarm_unicode_value,STR_UNI_SYS_ALARM_GONE);
							send_broadcast(STR_ALARM_SEND_BROADCAST_PRE, ACTION_SYS_ALARM_GONE, alarm.alarm_name);
						}
						InformUserByEmail(alarm);
						InformUserByWechat(alarm);
						InformUserByMobile(alarm);
					}
					break;
				case DEV_WENSHIDU:
					break;
				case DEV_SWITCH://get switch state
					{
						unsigned char cmd[6] = {0xFF};
						char *ip  =  strtok(NULL,",");
						char *p1 = strtok(NULL,",");
						char *p2 = strtok(NULL,",");
						char *p3 = strtok(NULL,",");
						if(ip == NULL || p1 == NULL || p2==NULL || p3 ==NULL){
							break;
						}
						cmd[0] = DEV_SWITCH;
						get_ip_byte( ip, cmd + 1);
						cmd[3] = strtol(p1, NULL, 16);
						cmd[4] = strtol(p2, NULL, 16);
						cmd[5] = strtol(p3, NULL, 16);
						DEBUG_MSG("%s: %.2X%.2X%.2X%.2X%.2X%.2X\n",__FUNCTION__,cmd[0],cmd[1],cmd[2],cmd[3],cmd[4],cmd[5]);
						uart_data_queuing(cmd, cmd_list_p[TYPE_SW_DEV].len);
					}
					break;
				case DEV_HONGWAI_REMOTE:
					break;
				default:
					break;
			}
		}
	}
	return NULL;
}


void SetDefaultAlarmUnicodeName(DEV_TYPE_LIST dev_type,char *unicode_name)
{
	switch(dev_type)
	{
		case DEV_SOUND:
			strcpy(unicode_name,STR_UNI_SOUND_SENSOR);
			break;
		case DEV_YUDI:
			strcpy(unicode_name,STR_UNI_YUDI_SENSOR);
			break;
		case DEV_YANWU:
			strcpy(unicode_name,STR_UNI_YANWU_SENSOR);
			break;
		case DEV_HONGWAI:
			strcpy(unicode_name,STR_UNI_HONGWAI_SENSOR);
			break;
		default:
			strcpy(unicode_name,STR_UNI_UNKNOWN_SENSOR);
			break;
	}
}

void SetDefaultAlarmName(DEV_TYPE_LIST dev_type,char *name)
{
	switch(dev_type)
	{
		case DEV_SOUND:
			strcpy(name,STR_CHS_SOUND_SENSOR);
			break;
		case DEV_YUDI:
			strcpy(name,STR_CHS_YUDI_SENSOR);
			break;
		case DEV_YANWU:
			strcpy(name,STR_CHS_YANWU_SENSOR);
			break;
		case DEV_HONGWAI:
			strcpy(name,STR_CHS_HONGWAI_SENSOR);
			break;
		default:
			strcpy(name,STR_CHS_UNKNOWN_SENSOR);
			break;
	}
}
