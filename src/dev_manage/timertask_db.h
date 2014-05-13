#ifndef __TIMERTASK_DB_H__
#define __TIMERTASK_DB_H__
#include "dev.h"

#define TIMERTASK_DB_NAME "/usr/dev/dev.db"

typedef struct{
	int timertask_id;
	char dev_mac[30];
	char dev_name[50];
	unsigned char period; // 1. 0000000 once 2. 0001 1111 3. 0111 1111 4. userdefine  
	char happen_time[20];
	unsigned char action; // 1. open 0. close
	unsigned int tipinfo; // (tipinfo&0xff)=tipway tipinfo&(0xFF00)=tiptime
	unsigned int howlong;
	unsigned char devnumber;
}timertask_item_t;

BOOL AddOneTimerTask(timertask_item_t item);
BOOL DeleteOneTimerTaskByID(int timertask_id);
void GetAllTimerTaskInfo(void);
void GetValidThingsToDo(const char *curr_time,const unsigned char weekday,unsigned char *nums,timertask_item_t *items);

#endif

