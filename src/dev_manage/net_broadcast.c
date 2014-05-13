#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include "net_broadcast.h"
#include <sys/types.h>
#include <string.h>

void send_broadcast(const char *who,USER_ACTION user_action,char *dev_name)
{
	struct sockaddr_in addr;
	int on = 1;
	socklen_t addr_len = sizeof(struct sockaddr_in);
	char msg[512] = {0};

	int socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if ( socket_fd < 0 ) {
		return;
	}

	if (setsockopt(socket_fd, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on)) < 0) {
		return;
	}

	memset(&addr, 0, sizeof(addr));
	addr.sin_family		= AF_INET;
	addr.sin_port 		= htons(BROADCAST_PORT);
	addr.sin_addr.s_addr= inet_addr("255.255.255.255");//htonl(INADDR_BROADCAST);

	//json send
	sprintf(msg,"[{\"who\":\"%s\",\"action\":\"%d\",\"dev_name\":\"%s\"}]",who,(int)(user_action),dev_name);
	
	sendto(socket_fd,msg,strlen(msg),0,(struct sockaddr *) &addr, addr_len);
}
