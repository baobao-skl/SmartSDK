#include <mac_helper.h>
#include <pthread.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#define RESPONSE_MSG "I am famint server!"
#define REQUEST_MSG "who is famint server?"

void *NetworkManagerThread(void *arg);
	   
int main(int argc,char **argv)
{
	pthread_t Network_Manager_ThreadId;
	pthread_create(&Network_Manager_ThreadId, NULL, NetworkManagerThread, NULL);
	pthread_detach(Network_Manager_ThreadId);
	pause();
	return 0;
}

void *NetworkManagerThread(void *arg)
{
	int s_server = 0;
	struct sockaddr_in serAddr; 
	struct sockaddr_in remoteAddr; 
	int nAddrLen = sizeof(remoteAddr);
	char recvData[255] = {0};

	s_server = socket(AF_INET, SOCK_DGRAM, 0);
	
	if(s_server < 0){
		return NULL;
	}

	serAddr.sin_family = AF_INET;  
	serAddr.sin_port = htons(8888);  
	serAddr.sin_addr.s_addr = INADDR_ANY;
	if(bind(s_server, (struct sockaddr *)&serAddr, sizeof(serAddr)) < 0)  
	{  
		close(s_server);  
		return 0;  
	}
	
	while (1)  
	{    
		memset(recvData, 0 , sizeof(recvData));
		if (recvfrom(s_server, recvData, 255, 0, (struct sockaddr *)&remoteAddr, &nAddrLen) > 0)  
		{ 
			printf("receive from:--%s--port=%d \r\n", inet_ntoa(remoteAddr.sin_addr),htons(remoteAddr.sin_port));  
			printf(recvData);
			printf("--\r\n"); 
			if(strstr(recvData,REQUEST_MSG)){
				printf("send data!!!!!!\n");
				sendto(s_server,RESPONSE_MSG,strlen(RESPONSE_MSG),0,(struct sockaddr *) &remoteAddr, nAddrLen);
			}     
		}     
	}  
	close(s_server);
}
