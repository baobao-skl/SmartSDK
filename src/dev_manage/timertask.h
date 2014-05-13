#ifndef __TIMER_TASK_H__
#define __TIMER_TASK_H__

#include <stdio.h>

#define TIMER_TIME 30//SECONDS

void *TimerTaskRun(void *arg);
void GetCurrentTime(char *current_time,unsigned char *weekday);
void TimeAdd(char *pre_time, int hour,int minutes,char *aftertime);
#endif
