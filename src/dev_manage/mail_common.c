#include "mail_common.h"
#include "mail.h"
#include <pthread.h>
#include "alarm_db.h"
#include <string.h>

#define SEND_MAIL_MOBAN_WITH_ATTACH 	"From: \"SmartHousehold\"<smart-household@center.com>\r\n"  \
										"To: \"user\"<smart-user@user.com>\r\n"  \
										"data\r\n" \
										"Subject: SmartHousehold System Alarm!!\r\n\r\n"  \
										"MIME-Version: 1.0\r\n" \
										"Content-Type: multipart/mixed;\r\n" \
										"\tboundary=\"XX-1234DED0099A\";\r\n" \
										"Content-Transfer-Encoding: 8bit\r\n\r\n" \
										"This is a multi-part message in MIME format.\r\n\r\n" \
										"--XX-1234DED0099A\r\n" \
										"Content-Type: text/plain;charset=us-ascii\r\n" \
										"Content-Transfer-Encoding: 8bit\r\n\r\n" \
										"%s %s!\r\n\r\n" \
										"--XX-1234DED0099A\r\n" \
										"Content-Type: image/jpg;\r\n" \
										"\tname=\"20140101163101.jpg\";\r\n" \
										"Content-Transfer-Encoding: base64\r\n" \
										"Content-Disposition: attachment; filename=\"img.jpg\"\r\n" \
										"%s\r\n\r\n" \
										"--XX-1234DED0099A\r\n.\r\n"

#define SEND_MAIL_MOBAN_NO_ATTACH		"From: \"SmartHousehold\"<smart-household@center.com>\r\n"  \
										"To: \"user\"<user@qq.com>\r\n"  \
										"Subject: SmartHousehold System Alarm!!\r\n\r\n"  \
										"%s %s!"
						

char body[1024*24] = {0};

void *send_mail_thread(void *arg)
{
	char ip[24] = {0};
	GetEmailList();
	char *email_list = get_email_list();
	if(strcmp(email_list,"_NULL_")==0)
		return NULL;//no email receiver

	//test network is OK!!
	if(GetIPByDomainName("www.baidu.com",ip)==FALSE){
		DEBUG_MSG("network is not ok now!\n");
		return NULL;
	}
	
	//if use 126 send fail,we will use QQ to send Email;
	if(SendMail(email_list, body,SERVER_QQ)==FALSE){
		if(SendMail(email_list, body,SERVER_126)==FALSE){
			SendMail(email_list, body,SERVER_SOHU);
		}
	}
	return NULL;
}

void InformUserByEmail(alarm_t alarm)
{
	pthread_t mail_ThreadId;
	sprintf(body,SEND_MAIL_MOBAN_WITH_ATTACH,alarm.alarm_name,alarm.alarm_value,"121");
	pthread_create(&mail_ThreadId, NULL, send_mail_thread, NULL);
	pthread_detach(mail_ThreadId);
}



