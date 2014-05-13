#ifndef __MUSIC_CONTROL_H__
#define __MUSIC_CONTROL_H__
#include <sys/types.h>
#include <stdlib.h>
#include "music.h"

#define MUSIC_START 				"PLAY_START"
#define MUSIC_STOP				"PLAY_STOP"
#define MUSIC_PAUSE		 		"PLAY_PAUSE"
#define MUSIC_CONTINUE 			"PLAY_CONTINUE"
#define GET_VOLUMN 				"GET_VOLUMN"
#define SET_VOLUMN 				"VOLUMN:"
#define CHMOD_MUSIC_FOLDER 		"CHMOD"
#define MUSIC_GET_LIST			"GET_MUSIC_LIST"
#define GET_PLAYER_STATE			"GET_PLAYER_STATE"
#define SET_PLAY_MODE			"SET_PLAY_MODE"

int GetCurrentVolumn(void);
void SetCurrentVolumn(int volumn);
void StartPlay(pid_t *childpid,char *filename);
void PlayMusic(char *filename);
void PlayPause(pid_t pid);
void PlayContinue(pid_t pid);
void PlayStop(pid_t *pid);
void ChmodMusicFolder(void);


void SetPlayerMode(PLAYER_MODE mode);
PLAYER_MODE GetPlayerMode(void);


void SetPlayerState(PLAYER_STATE state);
PLAYER_STATE GetPlayerState(void);
#endif
