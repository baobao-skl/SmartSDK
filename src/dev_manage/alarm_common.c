#include "alarm_common.h"

BOOL AddAlarmItem(ALARM_TYPE type,const char *value)
{
	if(isAlarmTypeAndValueMatch(type, value))//exist
	{
		return FALSE;
	}
	return insertIntoAlarmTable(type, value);
}

BOOL DeleteFromAlarmTable(ALARM_TYPE type,const char *value)
{
	if(isAlarmTypeAndValueMatch(type, value))//exist
	{
		return DeleteByEmailOrMobile(type,value);
	}
	return FALSE;
}

BOOL ChangeAlarmItemToState(ALARM_LOCK_ACTION lock_type,ALARM_TYPE type,const char *value)
{
	if(isAlarmTypeAndValueMatch(type, value))//exist
	{
		DEBUG_MSG("change state match\n");
		return SetAlarmState(lock_type, type,value);
	}
	DEBUG_MSG("change state dismatch");
	return FALSE;
}



