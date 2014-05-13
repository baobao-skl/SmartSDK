#include <stdio.h>  
#include <sys/types.h>  
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <errno.h>  
#include <unistd.h>  
#include <sys/time.h> 
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netdb.h>

#define WEB_DOMAIN "znjj.sinaapp.com"
#define BOOL unsigned char
#define TRUE 1
#define FALSE 0

#define WEB_REQUEST_URL 	"GET /upload.php?group_id=%s&type=%s&value=%s HTTP/1.1\r\n" \
						"Accept: text/html, application/xhtml+xml, */*\r\n" \
						"Accept-Language: zh-CN\r\n"  \
						"User-Agent: Mozilla/5.0 (compatible; MSIE 9.0; Windows NT 6.1; Trident/5.0)\r\n" \
						"Accept-Encoding: gzip, deflate\r\n" \
						"Host: znjj.sinaapp.com\r\n" \
						"Connection: Keep-Alive\r\n\r\n"

extern BOOL GetGroupID(char *group_id);
int open_socket(struct sockaddr *addr);
BOOL GetIPByDomainName(const char *domainName,char *ip);


void UploadToServer(const char*type,const char *value)
{
	char url[1024] = {0};
	char ip[20] = {0};
	int sockfd = 0; 
	int count = 0;
	struct sockaddr_in their_addr = {0};
	
	
	char group_id[1024] = {0};
	if(GetGroupID(group_id)==FALSE) return;
	
	if(GetIPByDomainName(WEB_DOMAIN,ip)==FALSE)
	{
		printf("Get IP Fail\n");
		return;
	}
	sprintf(url,WEB_REQUEST_URL,group_id,type,value);
	memset(&their_addr, 0, sizeof(their_addr));  
	their_addr.sin_family = AF_INET;  
	their_addr.sin_port = htons(80);
	their_addr.sin_addr.s_addr = inet_addr(ip);
	sockfd = open_socket((struct sockaddr *)&their_addr); 
	while(sockfd<0)
	{
		sockfd = open_socket((struct sockaddr *)&their_addr);
		if(count++>4)return;
	}
	printf("connect OK\n");
	printf("%s",url);
	int ret = send(sockfd, url, strlen(url), 0);
	printf("ret = %d\n",ret);
	memset(url,0,sizeof(url));
	recv(sockfd, url, 1500, 0);
	printf("recv = %s\n",url);
	close(sockfd);
}

BOOL GetIPByDomainName(const char *domainName,char *ip)
{
	struct hostent *hptr;
	char **pptr;
	char name[50] = {0};
	char str[20] = {0};
	strcpy(name,domainName);
	if((hptr = gethostbyname(name)) == NULL)
	{
		return FALSE;
	}
	switch(hptr->h_addrtype)
	{
		case AF_INET:
		case AF_INET6:
			pptr=hptr->h_addr_list;
			for(; *pptr!=NULL; pptr++){
				printf("address:%s\n",inet_ntop(hptr->h_addrtype, *pptr, str, sizeof(str)));
			}
			strcpy(ip,inet_ntop(hptr->h_addrtype, hptr->h_addr, str, sizeof(str)));
			return TRUE;
		default:
			break;
	}
	return FALSE;
}


int open_socket(struct sockaddr *addr)  
{  
	int sockfd = 0;
	struct timeval timeout = {30, 0};
	socklen_t len = sizeof(timeout);
	
	sockfd=socket(AF_INET, SOCK_STREAM, 0);  
	if(sockfd < 0)  
	{  
		printf("Open sockfd(TCP) error!\n");  
		return -1;  
	}  
	if (setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &timeout, len) == -1)
	{
		perror("setsockopt error");
		return -1;
	}
	if(connect(sockfd, addr, sizeof(struct sockaddr)) < 0)  
	{  
		printf("Connect sockfd(TCP) error!\n");
		close(sockfd);
		return -1;
	}  
	return sockfd;  
} 


