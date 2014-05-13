#ifndef __MAIL_COMMON_H__
#define __MAIL_COMMON_H__

#include "dev.h"

void InformUserByEmail(alarm_t alarm);
void *send_mail_thread(void *arg);

#endif
