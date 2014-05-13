#ifndef __SMS_CONTROL_H__
#define __SMS_CONTROL_H__
#include "sms.h"
#include "dev.h"

void handler_on_user_message(void);
int gsmDecode7bit(const char* pSrc, char* pDst, int nSrcLength);

typedef enum{
	SMS_ACTION_GET_LIST = 0,
	SMS_ACTION_CLOSE,
	SMS_ACTION_OPEN,
	SMS_ACTION_GET_STATE,
	SMS_ACTION_NULL,
	SMS_ACTION_MAX,
}SMS_ACTION;

#endif

