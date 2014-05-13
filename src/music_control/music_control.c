#include "music_control.h"
#include "music.h"
#include "play_list.h"
#include <stdio.h>
#include <string.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>

#define VOLUMN_ADJUST_CMD "amixer cset numid=6,iface=MIXER,name='PCM Playback Volume' "
#define GET_CURRENT_VOLUMN "amixer cget numid=6,iface=MIXER,name='PCM Playback Volume'"

extern char *p_addr;
extern int shmid;
extern pid_t childpid,grandpid;

void ChmodMusicFolder(void)
{
	char cmd[512] = {0};
	sprintf(cmd,"chmod 777 %s/* &",MUSIC_FOLDER);
	system(cmd);
}

int GetCurrentVolumn(void)
{
	char *line = NULL;
	char *volumn = NULL;
	size_t len;
	system(GET_CURRENT_VOLUMN " > /tmp/volumn");
	FILE  *fd  = fopen("/tmp/volumn","r");
	getline(&line,&len,fd);
	getline(&line,&len,fd);
	getline(&line,&len,fd);
	if(strtok(line,",")==NULL) return 0;
	fclose(fd);
	system("rm -rf /tmp/volumn");
	if((volumn = strtok(NULL,","))==NULL) return 0;
	return atoi(volumn);
}

void SetCurrentVolumn(int volumn)
{
	char volumn_cmd_line[1024] = {0};
	memset(volumn_cmd_line,0,sizeof(volumn_cmd_line));
	sprintf(volumn_cmd_line,"%s%d&",VOLUMN_ADJUST_CMD,volumn);
	system(volumn_cmd_line);
}

void StartPlay(pid_t *childpid,char *filename)
{
	pid_t pid = fork();
	if(pid > 0){
		*childpid = pid;
		sleep(1);
		memcpy(&grandpid,p_addr,sizeof(pid_t));
	}else if(0 == pid){
		PlayMusic(filename);
	}
}

void PlayMusic(char *filename)
{
	if(filename == NULL) return;
	music_list_item_t *curr_item = look_for_music(filename);
	pid_t temp_pid = 0;
	char *c_addr = NULL;
	PLAYER_STATE state = STATE_STOP;
	PLAYER_MODE mode = SINGAL_PLAY_MODE;
	while(curr_item != NULL)
	{
		temp_pid = fork();
		if(temp_pid == -1){
			//do nothing
			exit(1);
		}else if(temp_pid == 0){
			DEBUG_MSG("next song : %s\n",curr_item->item.name);
			char real_filepath[1024]={0};
			sprintf(real_filepath,"%s/%s",MUSIC_FOLDER,curr_item->item.name);
			state = STATE_PLAY;
			c_addr = shmat(shmid,0,0);
			memcpy(c_addr + sizeof(pid_t),&state,1);
			execl("/usr/bin/madplay","madplay",real_filepath,NULL);
		}else{
			c_addr = shmat(shmid,0,0);
			memcpy(c_addr,&temp_pid,sizeof(pid_t));
			memcpy(c_addr + sizeof(pid_t)+2,&curr_item,4);
			if(temp_pid == wait(NULL)){
				mode = GetPlayerMode();
				if(mode == SINGAL_PLAY_MODE){
					state = STATE_STOP;
					c_addr = shmat(shmid,0,0);
					memcpy(c_addr + sizeof(pid_t),&state,1);
					break;
				}else if(mode == SINGAL_LOOP_MODE){
					state = STATE_STOP;
				}else if(mode == ALL_LOOP_MODE){
					curr_item = curr_item->next;
					state = STATE_STOP;
				}
				c_addr = shmat(shmid,0,0);
				memcpy(c_addr + sizeof(pid_t),&state,1);
				memcpy(c_addr + sizeof(pid_t)+2,&curr_item,4);
			}
		}
	}
}

void PlayPause(pid_t pid)
{
	if(pid > 0){
		kill(pid,SIGSTOP);
		SetPlayerState(STATE_PAUSE);
	};
}

void PlayContinue(pid_t pid)
{
	if(pid > 0){
		kill(pid,SIGCONT);
		SetPlayerState(STATE_PLAY);
	}
}

void PlayStop(pid_t *pid)
{
	if(*pid > 0){
		kill(*pid,SIGKILL);
		*pid = 0;
	}
	if(childpid> 0){
		kill(childpid,SIGKILL);
		childpid = 0;
	}
	wait(NULL);
	SetPlayerState(STATE_STOP);
}

PLAYER_MODE GetPlayerMode(void)
{
	PLAYER_MODE mode = (PLAYER_MODE)(*((char *)(p_addr + sizeof(pid_t) + 1)));
 	return mode;
}

void SetPlayerMode(PLAYER_MODE mode)
{
	if(mode >= PLAY_MODE_MAX){
		mode = PLAY_MODE_MAX -1;
	}
	char *p_state = (char *)(p_addr + sizeof(pid_t) + 1);
	*p_state = mode;
	DEBUG_MSG("playmode = %d\n",*p_state);
}

void SetPlayerState(PLAYER_STATE state)
{
	char *p_state = (char *)(p_addr + sizeof(pid_t));
	*p_state = state;
}

PLAYER_STATE GetPlayerState(void)
{
	PLAYER_STATE state = (PLAYER_STATE)(*((char *)(p_addr + sizeof(pid_t))));
	return state;
}

