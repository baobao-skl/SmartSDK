#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<string.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<unistd.h>
#include<fcntl.h>
#include<pthread.h>

#define LOGIN_PORT 2002

void *new_loger_handler(void *arg);

typedef struct {
	char username[20];
	char password[33];
	char ip[15];
	char time[20];
	char type[10];
	pthread_t threadID;
	int fd;
}new_loger;

new_loger new_conn;

int main(int argc,char **argv)
{
	int on  = 1; 
	int len = 0;
	int server_fd = socket(AF_INET,SOCK_STREAM,0);
	struct sockaddr_in server_addr,client_addr;
	setsockopt(server_fd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));
	server_addr.sin_family		=	AF_INET;
	server_addr.sin_port			=	htons(LOGIN_PORT);
	server_addr.sin_addr.s_addr	=	INADDR_ANY;
	
	bzero(&(server_addr.sin_zero),8);
	bind(server_fd,(struct sockaddr *)&server_addr,sizeof(struct sockaddr));
	listen(server_fd,10);
	len = sizeof(struct sockaddr_in);
	while(1)
	{
		new_conn.fd = accept(server_fd,(struct sockaddr *)&client_addr,&len);
		pthread_create(&(new_conn.threadID),NULL,new_loger_handler,&new_conn);
		pthread_detach(new_conn.threadID);
	}
	return 0;
}


void *new_loger_handler(void *arg)
{
	char buffer[100]={0};
	new_loger *new_conn = arg;
	memset(buffer,0,sizeof(buffer));
	read(new_conn->fd,buffer,sizeof(buffer));
	sprintf(new_conn->username,"%s\0",strtok(buffer,"|"));
	sprintf(new_conn->password,"%s\0",strtok(NULL,"|"));
	sprintf(new_conn->ip,"%s\0",strtok(NULL,"|"));
	sprintf(new_conn->time,"%s\0",strtok(NULL,"|"));
	sprintf(new_conn->type,"%s\0",strtok(NULL,"|"));
	printf("username: %s\n",new_conn->username);
	printf("password: %s\n",new_conn->password);
	printf("ip:%s\n",new_conn->ip);
	printf("time:%s\n",new_conn->time);
	printf("type:%s\n",new_conn->type);
	write(new_conn->fd,"OK",2);
	close(new_conn->fd);
}