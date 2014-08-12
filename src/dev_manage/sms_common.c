#include "sms_common.h"
#include "sms.h"
#include <pthread.h>
#include <string.h>
#include "alarm_db.h"
#include "mail_common.h"
#include "dev.h"

extern sms_t sms;

message_info_t g_rcv_info;


char buffer[MAX_BUFFER_SIZE]={0};
char command[MAX_BUFFER_SIZE] ={0};
char rcv_msg_sim_index[3] = {0};
int read_length = 0;

//static void *InformUserByMobileAndEmailFunc(void *arg);

typedef void (*handler)(void);
void *sms_read_thread(void *arg)
{
	if (arg == NULL) {
		return NULL;
	}
	handler h = (handler)arg;
	while (TRUE) {
		if ((read_length=read(sms.fd,buffer,sizeof(buffer)))>0) {
			if (strstr(buffer,"0891")) {
				memset(&g_rcv_info,0,sizeof(&g_rcv_info));
				// 1. get the messgae info
				g_rcv_info = handler_receive_msg(strstr(buffer,"0891"));
				// 2.save log
				save_gsm_log(g_rcv_info);
				// 3.delete it from SIM
				memset(buffer,0,sizeof(buffer));
				sprintf(buffer,"AT+CMGD=%s\r",rcv_msg_sim_index);
				write(sms.fd,buffer,strlen(buffer));
				h();//to handler user message . wait next handler and dev control
			} else if (strstr(buffer,"CMTI:")) {//new message is coming
				memset(command,0,sizeof(command));
				strcpy(command,buffer);
				strtok(command,",");				
				memset(rcv_msg_sim_index,0,sizeof(rcv_msg_sim_index));
				strcpy(rcv_msg_sim_index,strtok(NULL,","));
				memset(buffer,0,sizeof(buffer));
				sprintf(buffer,"AT+CMGR=%s\r",rcv_msg_sim_index);
				write(sms.fd,buffer,strlen(buffer));  //read new message command
			}
			memset(buffer,0,sizeof(buffer));
		}
	}
}

void *send_msg_thread(void *arg)
{
	char *mobile;
	char temp[1024]={0};
	char content[1024] = {0};
	char tempMobile[12] = {0};
	alarm_t alarm;
	memset(&alarm,0,sizeof(&alarm));
	if (app_status.is_sms_ok==FALSE) return NULL;
	if (arg == NULL) return NULL;
	strcpy((char *)&alarm,arg);
	sprintf(content,"%s%s",alarm.alarm_unicode_name,alarm.alarm_unicode_value);
	GetMobileList();
	if (strcmp(get_mobile_list(),"_NULL_") == 0) return NULL;
	sprintf(temp,"%s",get_mobile_list());
	mobile = strtok(temp,",");
	while (mobile!=NULL) {
		DEBUG_MSG("%s\n",mobile);
		strcpy(tempMobile,mobile);
		sms.send(tempMobile,content);
		mobile = strtok(NULL,",");
		if (mobile == NULL) break;
		sleep(2);
	}
	return NULL;
}

void InformUserByMobile(alarm_t alarm)
{
	pthread_t mobile_ThreadId;
	alarm_t alarmtemp = alarm;
	pthread_create(&mobile_ThreadId, NULL, send_msg_thread, &alarmtemp);
	pthread_detach(mobile_ThreadId);
}

#if 0
void InformUserByMobileAndEmail(const )
{
	pthread_t mobileAndEmail_ThreadID;
	pthread_create(&mobileAndEmail_ThreadID, NULL, InformUserByMobileAndEmailFunc, NULL);
	pthread_detach(mobileAndEmail_ThreadID);
}

static void *InformUserByMobileAndEmailFunc(void *arg)
{
	send_mail_thread(NULL);
	send_msg_thread(NULL);
	return NULL;
}
#endif

