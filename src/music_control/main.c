#include "music.h"
#include "music_control.h"
#include <sys/ipc.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include "play_list.h"

#define PERM S_IRUSR|S_IWUSR

music_list_item_t *current_music = NULL;

music_list_item_t *music_head = NULL;
char *p_addr = NULL;

pid_t childpid = 0,grandpid= 0;
int lens = 0;
int shmid = 0;
char music_list[256*1024]= {0};
char music_state[1024] = {0};


char * GetMusicState(void);

int main(int argc,char **argv)
{
	int socket_server = 0 , socket_client = 0;
	fd_set fds;
	struct timeval timeout={0,0};
	struct sockaddr_in server_addr,client_addr;
	int  on = 1,maxfdp = 0;
	socklen_t sin_size = 0;
	unsigned char isSendContent = 0;
	char buffer[2*1024] = {0};
	
	//signal(SIGCHLD,SIG_IGN);

	//music_head = create_music_song(&lens);
	//if(music_head==NULL){
	//	DEBUG_MSG("create music song fail\n");
	//	exit(1);
	//}

	if((shmid = shmget(IPC_PRIVATE,5,PERM))== -1)
		exit(1);
	p_addr = shmat(shmid,0,0);
	memset(p_addr,'\0',1024);

	SetPlayerState(STATE_STOP);
	SetPlayerMode(SINGAL_LOOP_MODE);
	//output_all_music_info(music_head);
	//look_for_music("test3.mp3");
	//music_destory(music_head,lens);

	if((socket_server=socket(AF_INET,SOCK_STREAM,0))==-1)//create socket failure
	{
		perror("socket:");
		exit(1);
	}
	
	if (setsockopt(socket_server, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0) {
		DEBUG_MSG("setsockopt(SO_REUSEADDR) failed");
		exit(EXIT_FAILURE);
	}
	server_addr.sin_family		=	AF_INET;
	server_addr.sin_port			=	htons(MYPORT);    //setup the port that server can listen
	server_addr.sin_addr.s_addr	=	INADDR_ANY;  //setup the IP address
	
	bzero(&(server_addr.sin_zero),8);
	
	if(bind(socket_server,(struct sockaddr *)&server_addr,sizeof(struct sockaddr))==-1) //if bind socket failure
	{
		perror("bind");
		exit(1);
	}
	//if bind socket success,then setup the socket mode of listen and prepare to accept the request from client
	if(listen(socket_server,BACKLOG)==-1)
	{
		perror("listen");
		exit(1);
	}
	
	sin_size=sizeof(struct sockaddr_in);
	maxfdp = socket_server + 1;
	//if setup the listen mode success,then wait to the request from client and begin to handle it
	while(1)
	{	
		FD_ZERO(&fds);
		FD_SET(socket_server,&fds);
		switch(select(maxfdp,&fds,NULL,NULL,&timeout))
		{		
			case -1:
				break;
			case 0:
				break;
			default:
				if(FD_ISSET(socket_server,&fds))
				{
						if((socket_client=accept(socket_server,(struct sockaddr *)&client_addr,&sin_size))==-1)    
						{
							perror("accept");
							continue;
						}
						isSendContent= 0;
						memset(buffer,0,sizeof(buffer));
						recv(socket_client,buffer,sizeof(buffer),0);
						memcpy(&grandpid,p_addr,sizeof(pid_t));
						if(strstr(buffer,MUSIC_START)){
							char *filename = strchr(buffer,'+') + 1;
							PlayStop(&grandpid);
							StartPlay(&childpid,filename);
						}else if(strstr(buffer,SET_PLAY_MODE)){
							char *playmode = strchr(buffer,'+') + 1;
							SetPlayerMode(atoi(playmode));
						}else if(strstr(buffer,MUSIC_PAUSE)){
							PlayPause(grandpid);	
						}else if(strstr(buffer,MUSIC_CONTINUE)){
							PlayContinue(grandpid);
						}else if(strstr(buffer,MUSIC_STOP)){
							PlayStop(&grandpid);
						}else if(strstr(buffer,SET_VOLUMN)){
							strtok(buffer,":");
							SetCurrentVolumn(atoi(strtok(NULL,":")));
						}else if(strstr(buffer,GET_VOLUMN)){
							char volumn[10] = {0};
							sprintf(volumn,"%d",GetCurrentVolumn());
							send(socket_client,volumn,strlen(volumn),0);
							isSendContent = 1;
						}else if(strstr(buffer,CHMOD_MUSIC_FOLDER)){
							ChmodMusicFolder();
						}else if(strstr(buffer,MUSIC_GET_LIST)){
							if(music_head != NULL && lens > 0){
								DEBUG_MSG("[music] not first use!\n");
								music_destory(music_head,lens);
							}
							DEBUG_MSG("re create music list\n");
							music_head = create_music_song(&lens);
							memset(music_list, 0, sizeof(music_list));
							output_all_music_info(music_head,music_list);
							send(socket_client,music_list,strlen(music_list),0);
							isSendContent = 1;
						}else if(strstr(buffer,GET_PLAYER_STATE)){
							char *buffer = GetMusicState();
							send(socket_client,buffer,strlen(buffer),0);
							isSendContent = 1;
						}else{
							DEBUG_MSG("receive error command\n");	
						}

						if(isSendContent==0){
							if(send(socket_client,"OK\r\n",4,0)!=-1){
								DEBUG_MSG("send OK\n");
							}else{
								DEBUG_MSG("send fail\n");
							}
						}
						close (socket_client);
				}
		}
		usleep(1000);
	}
	close(socket_server);
	return 0;
}


char * GetMusicState(void)
{
	memcpy(&current_music,p_addr + sizeof(pid_t) + 2, 4);
	memset(music_state, 0 , sizeof(music_state));
	sprintf(music_state,"[{\"play_state\":\"%d\",\"play_mode\":\"%d\",\"music_name\":\"%s\",\"music_volumn\":\"%d\"}]",GetPlayerState(),GetPlayerMode(),current_music->item.name,GetCurrentVolumn());
	DEBUG_MSG("music_state: %s\n",music_state);
	return music_state;
}