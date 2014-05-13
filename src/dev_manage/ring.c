#include "ring.h"
#include <stdlib.h>
#include <stdio.h>

#define PLAYER_NAME "madplay"
#define MUSIC_PATH "/usr/alarm/"

ring_item ring_items[RING_MAX]={
	{"alarm.mp3"}
};

void system_ring(ring_type_t type)
{
	if(type > RING_MAX)
		return;
	char cmd[512] = {0};
	sprintf(cmd,"%s %s%s&",PLAYER_NAME,MUSIC_PATH,ring_items[type].path);
	system(cmd);
}




