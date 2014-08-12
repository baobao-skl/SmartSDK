#include "timertask.h"
#include <unistd.h>
#include <sys/time.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <stdlib.h>
#include "dev.h"
#include "timertask_db.h"
#include "uart_common.h"
#include "dev_control.h"

static void do_timer(void);
static void *do_timer_func(void *arg);

void *TimerTaskRun(void *arg)
{
	while(app_status.isTimerTaskRun)
	{
		if (app_status.isHasTimerTasks) {//this is a flag that has timer tasks
			do_timer();
		}
		// 4. wait
		usleep(TIMER_TIME*1000000*2);
	}
	return NULL;
}

static void do_timer(void)
{
	pthread_t do_timer_threadId;
	pthread_create(&do_timer_threadId, NULL, do_timer_func, NULL);
	pthread_detach(do_timer_threadId);
}

static void *do_timer_func(void *arg)
{
	unsigned char nums = 0,weekday = 0;
	timertask_item_t items[100];
	int i = 0;
	memset(items,0,sizeof(items));
	// 1.Get Current Time
	GetCurrentTime(app_status.CurrentTime,&weekday);
	// 2. Read database
	GetValidThingsToDo(app_status.CurrentTime,weekday,&nums,items);
	// 3. if timer is right, execute its task;
	if (nums > 0) {
		for (i = 0;i < nums; i++) {		
			ForwardControlToUart(items[i].dev_mac,items[i].dev_name,TYPE_SW_DEV,items[i].action,items[i].devnumber);
		}
	}
	return NULL;
}

void GetCurrentTime(char *current_time,unsigned char *weekday)
{
	struct tm		*timePassed;
	time_t		spentTime;
	time(&spentTime);
	timePassed = localtime(&spentTime);
	sprintf(current_time,"%.4d-%.2d-%.2d %.2d:%.2d",
						1900+timePassed->tm_year,
						1+timePassed->tm_mon,
						timePassed->tm_mday,		
						timePassed->tm_hour,
						timePassed->tm_min//,
						//timePassed->tm_sec
	);
	*weekday = (unsigned char)(timePassed->tm_wday);
	if (*weekday == 0) {
		*weekday = 7;//weekend 7
	}
}

void TimeAdd(char *pre_happen_time, int hour,int minutes,char *aftertime)
{
	struct tm pre_time_tm,*after_time_tm=NULL;
	time_t time1 = {0};
	char pre_time[20] = {0};
	strcpy(pre_time,pre_happen_time);
	DEBUG_MSG("pretime = %s\n",pre_time);
	char *pre1 = strtok(pre_time," ");
	char *pre2 = strtok(NULL," ");
	pre_time_tm.tm_year = atoi(strtok(pre1,"-")) - 1900;
	pre_time_tm.tm_mon = atoi(strtok(NULL,"-")) - 1;
	pre_time_tm.tm_mday = atoi(strtok(NULL,"-"));

	pre_time_tm.tm_hour = atoi(strtok(pre2,":"));
	pre_time_tm.tm_min = atoi(strtok(NULL,":"));
	pre_time_tm.tm_sec = 0;

	time1 = mktime(&pre_time_tm);

	time1 += (hour*60*60 + minutes*60);

	after_time_tm = localtime(&time1);
	sprintf(aftertime,"%.4d-%.2d-%.2d %.2d:%.2d",
						1900+after_time_tm->tm_year,
						1+after_time_tm->tm_mon,
						after_time_tm->tm_mday,		
						after_time_tm->tm_hour,
						after_time_tm->tm_min
	);
	DEBUG_MSG("addtime = H: %2d M: %2d\n",hour,minutes);
	DEBUG_MSG("aftertime = %s\n",aftertime);
}





