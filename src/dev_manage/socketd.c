#include <unistd.h>
#include <stdio.h>
#include "socketd.h"
#include <pthread.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include "dev.h"
#include "dev_del.h"
#include "dev_add.h"
#include "dev_edit.h"
#include "dev_db.h"
#include "sms.h"
#include "net_broadcast.h"
#include "cityid.h"
#include "uart_common.h"
#include "mail_common.h"
#include "alarm_common.h"
#include "sms_common.h"
#include "familygroup.h"
#include "led.h"
#include "timertask_db.h"
#include "timertask.h"
#include <cJSON/cJSON.h>

extern char g_timertask_info[1024*4];

extern sms_t sms;

void *client_thread( void *arg);

void *server_thread( void *arg ) 
{
	struct sockaddr_in addr, client_addr;
	int on = 1;
	pthread_t client;
	socklen_t addr_len = sizeof(struct sockaddr_in);
	int socket_fd = -1;
	int nNetTimeout = 10000;

	socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_fd < 0) {
		DEBUG_MSG("socket failed\n");
		exit(EXIT_FAILURE);
	}
	if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0) {
		DEBUG_MSG("setsockopt(SO_REUSEADDR) failed");
		exit(EXIT_FAILURE);
	}

	//发送时限
	setsockopt(socket_fd, SOL_SOCKET, SO_SNDTIMEO, (char *)&nNetTimeout ,sizeof(int));
	//接收时限
	setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO,(char *)&nNetTimeout, sizeof(int));

	memset(&addr, 0, sizeof(addr));
	addr.sin_family		= AF_INET;
	addr.sin_port 		= htons(DEV_MANAGE_PORT);
	addr.sin_addr.s_addr	= htonl(INADDR_ANY);
	if (bind(socket_fd, (struct sockaddr*)&addr, sizeof(addr)) != 0) {
		exit(EXIT_FAILURE);
	}

	/* start listening on socket */
	if (listen(socket_fd, 10)  != 0) {
		exit(EXIT_FAILURE);
	}

	/* create a child for every client that connects */
	while (1) {
		int fd = accept(socket_fd, (struct sockaddr *)&client_addr, &addr_len);
		if (pthread_create(&client, NULL, &client_thread, &fd) != 0) {
			close(fd);
			continue;
		}
		pthread_detach(client);
	}
	return NULL;
}

void *client_thread(void *arg)
{
	cJSON *root = NULL;
	char buffer[512] = {0};
	dev_tt dev;
	DEV_ADD_RETURN ret_add_dev = DEV_ADD_OK;
	BOOL ret = FALSE;
	char cityid[20] = {0};
	fd_set pending_data;
	struct timeval block_time;
	if (arg == NULL) {
		return NULL;
	}
	int *fd = arg;

	FD_ZERO(&pending_data);
	FD_SET(*fd,&pending_data);
	block_time.tv_sec = 5;
	block_time.tv_usec = 0;

	/*if client connect and not send message in 5 seconds,we will close the connection*/
	if (select((*fd) + 1, &pending_data, NULL, NULL, &block_time) > 0) {
		if (FD_ISSET(*fd, &pending_data)) {
			if (read(*fd, buffer, sizeof(buffer)) < 0) {
				close(*fd);
				return NULL;
			}
		}
	} 
	
	led_flash(LED_FOR_NETWORK, 1, 3);
	DEBUG_MSG("Network receive:%s\n", buffer);
	root = cJSON_Parse(buffer);//parse json
	if (root == NULL) {
		return NULL;
	}
	memset(&dev, 0, sizeof(dev_tt));
	char *who			= cJSON_GetObjectItem(root, "param0")->valuestring;
	dev.user_action 		= cJSON_GetObjectItem(root, "param1")->valuestring;
	dev.basic_info.mac		= cJSON_GetObjectItem(root, "param2")->valuestring;
	dev.basic_info.name 		= cJSON_GetObjectItem(root, "param3")->valuestring;
	dev.type 				= cJSON_GetObjectItem(root, "param4")->valuestring;
	dev.dev_cmd 			= cJSON_GetObjectItem(root, "param5")->valuestring;
	dev.basic_info.groupname	= cJSON_GetObjectItem(root, "param6")->valuestring;
	char *param1			= cJSON_GetObjectItem(root, "param7")->valuestring;
	char *param2			= cJSON_GetObjectItem(root, "param8")->valuestring;
	
	if (who == NULL || dev.user_action == NULL || dev.basic_info.mac==NULL ||dev.basic_info.name== NULL||dev.dev_cmd==NULL||dev.basic_info.groupname==NULL) {
		close(*fd);
		return NULL;
	}
	USER_ACTION user_action_type = (USER_ACTION)atoi(dev.user_action);
	switch (user_action_type) {
	case ACTION_DEV_ADD:
		dev.basic_info.name 		= strtok(dev.basic_info.name,":");//contains unicode
		dev.basic_info.unicode 	= strtok(NULL,":");
		dev.type = param2;
		if (dev.basic_info.name == NULL ||dev.basic_info.unicode == NULL) {
			break;
		}
		DEBUG_MSG("ADD:name= %s,unicode= %s\n",dev.basic_info.name,dev.basic_info.unicode);
		ret_add_dev = AddDevToStaticTable(dev.basic_info.mac,dev.basic_info.name,dev.basic_info.unicode,dev.basic_info.groupname,atoi(param1),atoi(dev.type));
		if (ret_add_dev==DEV_ADD_OK) {
			write(*fd, SOCKET_RESPONSE_YES, strlen(SOCKET_RESPONSE_YES));
			send_broadcast(who, user_action_type, dev.basic_info.name);
		} else if (ret_add_dev == DEV_MAC_EXIST) {
			write(*fd,SOCKET_RESPONSE_MAC_EXIST, strlen(SOCKET_RESPONSE_MAC_EXIST));
		} else if (ret_add_dev == DEV_NAME_EXIST) {
			write(*fd,SOCKET_RESPONSE_NAME_EXIST, strlen(SOCKET_RESPONSE_NAME_EXIST));
		}
		break;
	case ACTION_DEV_REMOVE:
		ret = RemoveDevFromStaticTable(dev.basic_info.mac,dev.basic_info.name);
		if (ret) {
			send_broadcast(who,user_action_type,dev.basic_info.name);
		}
		goto response_client;
		break;
	case ACTION_DEV_ADD_GROUP:
		ret = AddDevGroupName(dev.basic_info.groupname);
		goto response_client;
		break;
	case ACTION_DEV_EDIT:
		{
			char *oldname = param1;
			dev.basic_info.name = strtok(dev.basic_info.name,":");
			dev.basic_info.unicode = strtok(NULL,":");
			if (dev.basic_info.name == NULL ||dev.basic_info.unicode == NULL||oldname==NULL) {
				break;			
			}
			DEBUG_MSG("UPDATE:oldname = %s newname= %s,unicode= %s\n",oldname,dev.basic_info.name,dev.basic_info.unicode);
			ret = UpdateDevDisplayName(dev.basic_info.mac, oldname,dev.basic_info.name,dev.basic_info.unicode);
			if (ret) {
				send_broadcast(who,user_action_type,dev.basic_info.name);
			}
			goto response_client;
		}
		break;
	case ACTION_GET_ALARM_LIST:
		GetAlarmAllList();
		write(*fd,get_alarm_alllist(),strlen(get_alarm_alllist()));
		break;
	case ACTION_GET_DEV_LIST:
		GetDeviceList();
		write(*fd,get_dev_list(),strlen(get_dev_list()));
		break;
	case ACTION_GET_CITY_ID:
		//char *name = dev.basic_info.name;
		memset(cityid,0,sizeof(cityid));
		if (GetCityIDByName(dev.basic_info.name, cityid)) {
			write(*fd,cityid,strlen(cityid));
		} else {
			write(*fd,SOCKET_RESPONSE_NO,strlen(SOCKET_RESPONSE_NO));
		}
		break;
	case ACTION_DEV_CONTROL:
		if (app_status.is_uart_ok) {
			ret = ForwardControlToUart(dev.basic_info.mac,dev.basic_info.name,(DEV_TYPE_INDEX)(atoi(dev.type)),atoi(dev.dev_cmd),atoi(param1));
		} else {
			ret = FALSE;
		}
		goto response_client;
		break;
	case ACTION_SEND_MSG:
		if (app_status.is_sms_ok) {
			ret = sms.send(dev.basic_info.mac,dev.basic_info.name);
		} else {
			ret = FALSE;
		}
		goto response_client;
		break;
	case ACTION_ALARM_ADD:
		//mac as the value(email or mobile value)
		ret = AddAlarmItem((ALARM_TYPE)(atoi(dev.type)), dev.basic_info.mac);
		goto response_client;
		break;
	case ACTION_ALARM_LOCK:
		ret = ChangeAlarmItemToState(ALARM_LOCK,(ALARM_TYPE)(atoi(dev.type)), dev.basic_info.mac);
		goto response_client;
		break;
	case ACTION_ALARM_UNLOCK:
		ret = ChangeAlarmItemToState(ALARM_UNLOCK,(ALARM_TYPE)(atoi(dev.type)), dev.basic_info.mac);
		goto response_client;
		break;
	case ACTION_ALARM_DELETE:
		ret = DeleteFromAlarmTable((ALARM_TYPE)(atoi(dev.type)), dev.basic_info.mac);
		goto response_client;
		break;
	case ACTION_GET_ALL_GROUP_NAME:
		GetAllGroupName();
		write(*fd , get_group_list(), strlen(get_group_list()));
		break;
	case ACTION_UPDATE_DEV_GROUP:
		ret = UpdateDevGroupName(dev.basic_info.mac,dev.basic_info.name,dev.basic_info.groupname);
		goto response_client;
		break;
	case ACTION_SET_GROUP_ID:
		{
			groupInfo_t group_info;
			memset(&group_info,0,sizeof(&group_info));
			strcpy(group_info.group_id,dev.basic_info.mac);
			strcpy(group_info.group_name,dev.basic_info.name);
			ret = SetFamilyGroupInfo(group_info);
			goto response_client;
		}
		break;
	case ACTION_GET_GROUP_ID:
		{
			groupInfo_t group_info;
			memset(&group_info,0,sizeof(&group_info));
			if (GetFamilyGroupInfo(&group_info)) {
					char buf[1024] = {0};
					sprintf(buf,"%s,%s,",group_info.group_id,group_info.group_name);
					write(*fd,buf,strlen(buf));
			} else {
				write(*fd,"_FAIL_",strlen("_FAIL_"));
			}
		}
		break;
	case ACTION_ADD_TIMERTASK:
		{
			timertask_item_t item;
			DEBUG_MSG("add timertask!\n");
			memset(&item,0,sizeof(&item));
			strcpy(item.dev_mac,dev.basic_info.mac);
			strcpy(item.dev_name,dev.basic_info.name);
			strcpy(item.happen_time ,param1);
			item.period		=	atoi(dev.type);
			item.action		=	atoi(dev.dev_cmd);
			item.tipinfo		=	atoi(dev.basic_info.groupname);
			item.howlong		=	atoi(param2);
			DEBUG_MSG("action = %d, howlong = hour:%2d,minute:%2d\n",item.action,(item.howlong)/100,(item.howlong)%100);
			ret = AddOneTimerTask(item);
			if (ret && item.howlong > 0) {
				TimeAdd(item.happen_time,item.howlong/100, item.howlong%100, item.happen_time);
				if (item.action == CMD_SW_OPEN) {
					item.action = CMD_SW_CLOSE;
				} else {
					item.action = CMD_SW_OPEN;
				}
				item.howlong = 0;//avoid repeat
				ret = AddOneTimerTask(item);
			}
			goto response_client;
		}
		break;
	case ACTION_DELETE_TIMERTASK:
		{
			int timertask_id = 0;
			ret = DeleteOneTimerTaskByID(timertask_id);
			goto response_client;
		}
		break;
	case ACTION_GET_TIMERTASK_INFO:
		GetAllTimerTaskInfo();
		write(*fd,g_timertask_info,strlen(g_timertask_info));
		break;
	case ACTION_SW_GET_STATE:
		{
			char buffer[10] = {0};
			enum control_cmd state = SWGetCurrentState(dev.basic_info.mac,dev.basic_info.name,atoi(param1));
			sprintf(buffer,"%d", state);
			write(*fd,buffer,strlen(buffer));
		}
		break;
	case ACTION_QUIT:
		close(*fd);
		return NULL;
	default:
		break;
	}
	close(*fd);
	return NULL;
response_client:
	if (ret) {
		write(*fd, SOCKET_RESPONSE_YES, strlen(SOCKET_RESPONSE_YES));
	} else {
		write(*fd, SOCKET_RESPONSE_NO, strlen(SOCKET_RESPONSE_NO));
	}
	close(*fd);
	return NULL;
}

