#ifndef __SMS_COMMON_H__
#define __SMS_COMMON_H__

#include "dev.h"

void *sms_read_thread(void *arg);
void *send_msg_thread(void *arg);
void InformUserByMobile(alarm_t alarm);
//void InformUserByMobileAndEmail(void);

#endif
