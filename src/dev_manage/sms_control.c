#include "sms_control.h"
#include "alarm_db.h"
#include <pthread.h>
#include "sms.h"
#include "dev_db.h"
#include <string.h>
#include <stdlib.h>
#include "dev_control.h"

extern message_info_t g_rcv_info;
extern sms_t sms;

static void *user_msg_process(void *arg);
static void send_dev_list_by_sms(const char *who);

void handler_on_user_message(void)
{
	pthread_t handler_sms_threadID;
	pthread_create(&handler_sms_threadID, NULL, user_msg_process, &g_rcv_info);
	pthread_detach(handler_sms_threadID);
}

static void *user_msg_process(void *arg)
{
	if(arg == NULL) 
		return NULL;
	message_info_t *msg_info = arg;
	SMS_ACTION sms_action = SMS_ACTION_NULL;
	char user_msg_decode[100] = {0};
	char ip[10] = {0};
	unsigned char devnumber = 0;
	if(isUserMobileNumberValid(msg_info->send_number)==FALSE){
		DEBUG_MSG("invalid number from :%s\n",msg_info->send_number);
		return NULL;
	}
	DEBUG_MSG("valid number from :%s\n",msg_info->send_number);
	switch(msg_info->charset)
	{
		case CHARSET_7BITS://mainly use to send english
			if(strlen(msg_info->send_content)>100) return NULL;
			if(gsmDecode7bit(msg_info->send_content,user_msg_decode,strlen(msg_info->send_content))<0)
				return NULL;
			DEBUG_MSG("user_msg_decode = %s\n",user_msg_decode);
			if(strspn(user_msg_decode,"0123456789")<strlen(user_msg_decode)){
				DEBUG_MSG("Invalid characters contains!!\n");
				return NULL;
			}
			if(strcmp(user_msg_decode,"909")==0){
				sms_action = SMS_ACTION_GET_LIST;
			}else{
				char id[20] = {0};
				substring(user_msg_decode,id,0,strlen(user_msg_decode)-1);
				DEBUG_MSG("id = %s--\n",id);
				if(GetIPAndDevNumberByID(id,ip,&devnumber)==FALSE){
					return NULL;
				}
				DEBUG_MSG("ip = %s--\n",ip);
				sms_action  = (SMS_ACTION)(user_msg_decode[strlen(user_msg_decode)-1] - 0x30 + 1);
				if(sms_action > SMS_ACTION_MAX -2){
					return NULL;
				}
				DEBUG_MSG("action = %d--\n",sms_action);
			}
			break;
		default:
			break;
	}
	switch(sms_action)
	{
		case SMS_ACTION_GET_LIST:
			send_dev_list_by_sms(msg_info->send_number);
			break;
		case SMS_ACTION_CLOSE:
			switch_dev_control_func(ip,CMD_SW_CLOSE,devnumber);
			break;
		case SMS_ACTION_OPEN:
			switch_dev_control_func(ip,CMD_SW_OPEN,devnumber);
			break;
		case SMS_ACTION_GET_STATE:
			switch_dev_control_func(ip,CMD_SW_GET_STATE,devnumber);
			break;
		default:
			break;
	}
	return NULL;
}

static void send_dev_list_by_sms(const char *who)
{
	char temp[16*1024] = {0};
	GetUnicodeDeviceList();
	char *unicode_dev_list = get_unicode_dev_list();
	DEBUG_MSG("who = %s ,len= %d, %s\n",who,strlen(unicode_dev_list),unicode_dev_list);
	if(strcmp(unicode_dev_list,"_NULL_")==0){//对不起,列表为空!
		sms.send(who,"5BF94E0D8D77002C8FDC7A0B8BBE5907521788684E3A7A7A0021");
		return;
	}
	strncpy(temp,unicode_dev_list,strlen(unicode_dev_list));
	sleep(2);
	sms.send(who,temp);
}

int gsmDecode7bit(const char* Src, char* pDst, int nSrcLength)
{
	int nSrc = 0;
	int nDst = 0;
	int nByte = 0;
	unsigned char nLeft = 0;
	
	unsigned char *pSrc = NULL;
	unsigned char x[50] = {0};
	pSrc = (unsigned char *)&x;
	
	if(nSrcLength%2!=0) return -1;
	
	int i = 0;
	char y[3] = {0};
	for(i = 0;i < nSrcLength;i++){
		substring(Src,y,2*i,2);
		*pSrc = strtol(y,NULL,16);
		pSrc++;
	}

	pSrc =  (unsigned char *)&x; //restore point
	
	while(nSrc<nSrcLength)
	{
		*pDst = ((*pSrc << nByte) | nLeft) & 0x7f;
		nLeft = *pSrc >> (7-nByte);
		pDst++;
		nDst++;
		nByte++;
		if(nByte == 7)
		{
			*pDst = nLeft;
			pDst++;
			nDst++;
			nByte = 0;
			nLeft = 0;
		}
		pSrc++;
		nSrc++;
	}
	*pDst = '\0';
	return nDst;
}