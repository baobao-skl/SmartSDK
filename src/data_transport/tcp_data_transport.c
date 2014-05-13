#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<string.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<sys/select.h>
#include<sys/wait.h>
#include<sys/time.h>
#include<unistd.h>
#include<fcntl.h>
#include<signal.h>

#define MYPORT 2000 //setup the port the server
#define BACKLOG 10 //setup the maximum numbers that clients can link 
#define BUFFERSIZE 20

int main(int argc,char **argv)
{
	int socket_server ,socket_client;
	int result,i;
	unsigned char wenduValue[5];
	int shmid=0;
	char *shm_addr_main;
	char *shm_addr_child;
	int ds18b20_fd;
	
	fd_set fds;
	struct timeval timeout={0,0};
	int maxfdp;
	
	struct sockaddr_in server_addr;
	struct sockaddr_in client_addr;
	int sin_size;
	int on=1;
	char sendString[40];
	char buffer[100];
	/*
	1.create socket
	2.bind IP address and port fort for socket
	3.start to listen request from client
	4.accept the request from client and handle it
	*/
	if((socket_server=socket(AF_INET,SOCK_STREAM,0))==-1)//create socket failure
	{
		perror("socket");
		exit(1);
	}
	setsockopt(socket_server,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));
	//if create socket success,then setup port and IP address for server 
	server_addr.sin_family=AF_INET;
	server_addr.sin_port=htons(MYPORT);    //setup the port that server can listen
	server_addr.sin_addr.s_addr=INADDR_ANY;  //setup the IP address
	
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
	if((ds18b20_fd=open("/dev/dht11",0))<0)
	{
	perror("open ds18b20:");		
	exit(1);	
	}
	//create share-memory
	if((shmid=shmget(IPC_PRIVATE,BUFFERSIZE,0666))<0)
	{
		perror("create share-memory");
		exit(1);
	}
	if((shm_addr_main = shmat(shmid,0,0))==(void *)-1)
	{
		perror("Parent: shmat:");
		exit(1);
	}
	
	signal(SIGCHLD,SIG_IGN);//avoid jiangshi process
	
	sin_size=sizeof(struct sockaddr_in);
	maxfdp = socket_server + 1;
	//if setup the listen mode success,then wait to the request from client and begin to handle it
	while(1)
	{	
		if(read(ds18b20_fd,(char *)wenduValue,5)>0){
			sprintf(buffer,"[{wendu:\"%d\",shidu:\"%d\"}]",wenduValue[2]*10+wenduValue[3]/100,wenduValue[0]);
			strcpy(shm_addr_main,buffer);
		}
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
						//if client link failure
						if((socket_client=accept(socket_server,(struct sockaddr *)&client_addr,&sin_size))==-1)    
						{
							perror("accept");
							continue;
						}
						printf("server:got connection from %s\n",inet_ntoa(client_addr.sin_addr));
						if(!fork())
						{
							if((shm_addr_child = shmat(shmid,0,0))==(void *)-1)
							{
								perror("Child: shmat:");
								exit(1);
							}
							while(i!=-1)
							{
								strcpy(sendString,shm_addr_child);
								i=send(socket_client,sendString,strlen(sendString),0);
								usleep(600000);
							}
							perror("send");
							close (socket_client);
							exit(1);
						}
				}
		}
		usleep(500000);
	}
	close(ds18b20_fd);
	close(socket_server);
	shmctl(shmid,IPC_RMID,NULL);
	while(waitpid(-1,NULL,WNOHANG)>0);
	return 0;
}
