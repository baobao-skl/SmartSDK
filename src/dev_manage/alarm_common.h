#ifndef __ALARM_COMMON_H__
#define __ALARM_COMMON_H__

#include "dev.h"
#include "alarm_db.h"


BOOL AddAlarmItem(ALARM_TYPE type,const char *value);
BOOL DeleteFromAlarmTable(ALARM_TYPE type,const char *value);
BOOL ChangeAlarmItemToState(ALARM_LOCK_ACTION lock_type,ALARM_TYPE type,const char *value);

#endif
