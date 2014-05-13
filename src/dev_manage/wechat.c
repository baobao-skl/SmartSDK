#include "wechat.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "dev.h"

void InformUserByWechat(alarm_t alarm)
{
	char cmd[512] = {0};
	sprintf(cmd,"upload %s %s&",alarm.alarm_name,alarm.alarm_value);
	system(cmd);
}

