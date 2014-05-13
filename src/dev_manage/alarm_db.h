#ifndef __ALARM_DB_H__
#define __ALARM_DB_H__
#include "dev.h"

#define USER_DB_NAME "/usr/dev/user.db"


typedef enum{
	ALARM_TYPE_EMAIL = 0,
	ALARM_TYPE_MOBILE,
}ALARM_TYPE;

typedef enum{
	ALARM_LOCK = 0,
	ALARM_UNLOCK
}ALARM_LOCK_ACTION;

BOOL isUserMobileNumberValid(const char *mobile_number);
BOOL insertIntoAlarmTable(ALARM_TYPE type,const char *value);
BOOL isAlarmTypeAndValueMatch(ALARM_TYPE type,const char *vaue);
BOOL DeleteByEmailOrMobile(ALARM_TYPE alarm_type,const char *value);
BOOL SetAlarmState(ALARM_LOCK_ACTION lock_type,ALARM_TYPE type,const char *value);
void GetEmailList(void);
void GetMobileList(void);
void GetAlarmAllList(void);
char * get_alarm_alllist(void);
char * get_mobile_list(void);
char * get_email_list(void);

#endif
