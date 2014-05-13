#include "TimerTask.h"
#include <unistd.h>
#include <sys/time.h>
#include <string.h>
#include <time.h>

void *TimerTaskRun(void *arg)
{
	printf("timer start!!!!!!!!!!");
	while(1)
	{
		printf("timeout!!\n");
		// 1.Get Current Time
		
		// 2. Read database

		// 3. if timer is right, execute its task;
		
		// 4. wait
		sleep(TIMER_TIME);
	}
	return NULL;
}

void GetCurrentTime(char *current_time)
{
	struct tm		*timePassed;
	time_t		spentTime;
	time(&spentTime);
	timePassed = localtime(&spentTime);
	sprintf(current_time,"%.4d-%.2d-%.2d %.2d:%.2d:%.2d",
						1900+timePassed->tm_year,
						1+timePassed->tm_mon,
						timePassed->tm_mday,		
						timePassed->tm_hour,
						timePassed->tm_min,
						timePassed->tm_sec
	);
}





