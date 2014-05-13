#include "TimerTask.h"
#include <pthread.h>
#include <unistd.h>

char current_time[15] = {0};

int main(int argc,char **argv)
{
	
	pthread_t Timer_ThreadId;
	
	GetCurrentTime(current_time);
	printf("current-time: %s\n",current_time);
	
	pthread_create(&Timer_ThreadId, NULL, TimerTaskRun, NULL);
	pthread_detach(Timer_ThreadId);
	pause();
	return 0;
}