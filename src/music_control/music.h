#ifndef __MUSIC_H__
#define __MUSIC_H__

#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define DEBUG 1
#define DEBUG_MSG if(DEBUG)printf

#define MUSIC_FOLDER "/usr/ftp/music"
#define MYPORT 2001 //setup the port the server
#define BACKLOG 10 //setup the maximum numbers that clients can link 

typedef struct{
	char name[1024];
	char author[512];
	char length[20];
}music_basic_item_t;

typedef struct music_list_item{
	music_basic_item_t item;
	struct music_list_item *prev;
	struct music_list_item *next;
}music_list_item_t;

typedef enum{
	STATE_PLAY,
	STATE_PAUSE,
	STATE_STOP
}PLAYER_STATE;

typedef enum{
	SINGAL_PLAY_MODE = 0,
	SINGAL_LOOP_MODE,
	ALL_LOOP_MODE,
	PLAY_MODE_MAX
}PLAYER_MODE;

extern pid_t pid,grandpid;

#endif
