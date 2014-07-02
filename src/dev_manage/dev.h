#ifndef __DEV_H__
#define __DEV_H__

#include <stdio.h>

#define BOOL unsigned char
#define TRUE 1
#define FALSE 0


#define DEBUG
#ifdef DEBUG
	#define DEBUG_MSG printf
#else
	#define DEBUG_MSG(...)
#endif
	
//2014-09-09 09:00
typedef struct{
	BOOL is_sms_ok;
	BOOL is_uart_ok;
	BOOL isTimerTaskRun;
	char CurrentTime[20];
}app_status_t;

extern app_status_t app_status;

typedef struct{
	char alarm_name[512];
	char alarm_value[512];
	char alarm_unicode_name[512];
	char alarm_unicode_value[512];
}alarm_t;

typedef struct{
	char *mac;
	char *name;
	char *ip;
	char *unicode;
	char *groupname;
}dev_basic_t;

typedef struct{
	dev_basic_t basic_info; 
	char *user_action;
	char *type;
	char *dev_cmd;
}dev_tt;

enum control_cmd{
	CMD_SW_OPEN,
	CMD_SW_CLOSE,
	CMD_SW_TOGGLE,
	CMD_SW_GET_STATE,
	CMD_CL_OPEN,
	CMD_CL_CLOSE,
	CMD_CL_PAUSE,
	CMD_CL_GET_STATE,
	CMD_DOOR_OPEN,
	CMD_DOOR_CLOSE,
	CMD_DOOR_GET_STATE,
	CMD_HONGWAI_ADD,
	CMD_HONGWAI_DEL,
	CMD_HONGWAI_SEND,
	CMD_NULL
};

typedef enum type_index{
	TYPE_SW_DEV=0,
	TYPE_CL_DEV,
	TYPE_DOOR_DEV,
	TYPE_KONGTIAO_DEV,
	TYPE_WENSHIDU_DEV,
	TYPE_SOUND_DEV,
	TYPE_HONGWAI_DEV,
	TYPE_YANWU_DEV,
	TYPE_YUDI_DEV,
	TYPE_INDEX_MAX
}DEV_TYPE_INDEX;/*use for network and server*/

typedef enum{
	DEV_NULL = 0xFF,
	DEV_ADD = 0x01,
	DEV_SWITCH= 0x02,
	DEV_CHUANGLIAN = 0x03,
	DEV_DOOR = 0x04,
	DEV_HONGWAI_REMOTE = 0x05,
	DEV_YUDI = 0x06,
	DEV_WENSHIDU = 0x07,
	DEV_SOUND = 0x08,
	DEV_HONGWAI = 0x09,
	DEV_YANWU = 0x10,
	
}DEV_TYPE_LIST;

typedef enum{
	ACTION_DEV_ADD=0,
	ACTION_DEV_REMOVE,
	ACTION_DEV_EDIT,
	ACTION_GET_DEV_LIST,
	ACTION_DEV_CONTROL,
	ACTION_SEND_MSG,
	ACTION_SYS_ALARM_COME,
	ACTION_SYS_ALARM_GONE,
	ACTION_GET_CITY_ID,
	ACTION_ALARM_ADD,
	ACTION_GET_ALARM_LIST,
	ACTION_ALARM_LOCK,
	ACTION_ALARM_UNLOCK,
	ACTION_ALARM_DELETE,
	ACTION_DEV_ADD_GROUP,
	ACTION_GET_ALL_GROUP_NAME,
	ACTION_UPDATE_DEV_GROUP,
	ACTION_SET_GROUP_ID,
	ACTION_GET_GROUP_ID,
	ACTION_ADD_TIMERTASK,
	ACTION_DELETE_TIMERTASK,
	ACTION_GET_TIMERTASK_INFO,
	ACTION_SW_GET_STATE,
	ACTION_MAX
}USER_ACTION;

typedef enum{
	DEV_MAC_EXIST = 0,
	DEV_NAME_EXIST,
	DEV_ADD_OK,
	DEV_ADD_FAIL
}DEV_ADD_RETURN;


#endif


