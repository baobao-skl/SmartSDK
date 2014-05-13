#ifndef __TIMER_TASK_H__
#define __TIMER_TASK_H__

#include <stdio.h>

#define TIMER_TIME 5//SECONDS

void *TimerTaskRun(void *arg);
void GetCurrentTime(char *current_time);

#endif
