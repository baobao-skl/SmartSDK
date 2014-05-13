#include "mail.h"
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
#include "dev.h"

static char con628(char c6);  
static void base64(char *dbuf,char *buf128,int len); 
static int open_socket(struct sockaddr *addr); 


MAIL_SERVER mail_server_list[] ={
		//{"smtp.qq.com",25,"smart_household@qq.com","192.168.0.1,.,."},
		{"smtp.qq.com",25,"1010622394@qq.com","d150b9320b9304.."},
		{"smtp.126.com",25,"smart_household@126.com","192.168.0.1.,.,"},
		{"smtp.sohu.com",25,"smart-household@sohu.com","192.168.0.1;;;;"}
};

/*
EHLO licp
AUTH LOGIN
ZDE1MDkzMjA5MzA0QDEyNi5jb20=
MTE4NDUyLi4=
MAIL FROM:<d15093209304@126.com>
RCPT TO:<1010622394@qq.com>
*/

BOOL SendMail(char *to, char *body,MAIL_SERVER_ITEM server_type)  
{  
	int sockfd = 0;  
	struct sockaddr_in their_addr = {0};  
	char buf[1500] = {0};  
	char rbuf[1500] = {0};  
	char login[128] = {0};  
	char pass[128] = {0};
	char userlist[1024] = {0};
	char *userto;

	char ip[20] = {0};
	if(GetIPByDomainName(mail_server_list[server_type].name,ip)==FALSE)
		return FALSE;
	DEBUG_MSG("ip = %s,len = %d\n",ip,strlen(ip));
	memset(&their_addr, 0, sizeof(their_addr));  
	their_addr.sin_family = AF_INET;  
	their_addr.sin_port = htons(mail_server_list[server_type].port);
	their_addr.sin_addr.s_addr = inet_addr(ip);
	sockfd = open_socket((struct sockaddr *)&their_addr);  
	if(sockfd<0)return FALSE;
	memset(rbuf,0,1500);  
	while(recv(sockfd, rbuf, 1500, 0) == 0)  
	{  
		sleep(2);  
		close(sockfd);  
		sockfd = open_socket((struct sockaddr *)&their_addr);  
		memset(rbuf,0,1500);  
	}  
	// EHLO  
	DEBUG_MSG("%s\n", rbuf);  
	memset(buf, 0, 1500);  
	sprintf(buf, "EHLO licp\r\n");  
	send(sockfd, buf, strlen(buf), 0);  
	memset(rbuf, 0, 1500);  
	recv(sockfd, rbuf, 1500, 0); 
	DEBUG_MSG("%s\n", rbuf);  
	// AUTH LOGIN  
	memset(buf, 0, 1500);  
	sprintf(buf, "AUTH LOGIN\r\n");  
	send(sockfd, buf, strlen(buf), 0);  
	memset(rbuf, 0, 1500);  
	recv(sockfd, rbuf, 1500, 0);  
	DEBUG_MSG("%s\n", rbuf);  
	// USER  
	memset(buf, 0, 1500);  
	sprintf(buf,mail_server_list[server_type].username);  
	memset(login, 0, 128);  
	base64(login, buf, strlen(buf));  
	sprintf(buf, "%s\r\n", login);  
	send(sockfd, buf, strlen(buf), 0);   
	memset(rbuf, 0, 1500);  
	recv(sockfd, rbuf, 1500, 0);
	DEBUG_MSG("%s\n", rbuf);  
	// PASSWORD  
	sprintf(buf, mail_server_list[server_type].password);  
	memset(pass, 0, 128);  
	base64(pass, buf, strlen(buf));  
	sprintf(buf, "%s\r\n", pass);  
	send(sockfd, buf, strlen(buf), 0);  
	DEBUG_MSG("%s\n", buf);  
	memset(rbuf, 0, 1500);  
	recv(sockfd, rbuf, 1500, 0);
	DEBUG_MSG("%s\n", rbuf);  
	// MAIL FROM  
	memset(buf, 0, 1500);  
	sprintf(buf, "MAIL FROM: <%s>\r\n",mail_server_list[server_type].username);  
	send(sockfd, buf, strlen(buf), 0);  
	memset(rbuf, 0, 1500);  
	recv(sockfd, rbuf, 1500, 0); 
	DEBUG_MSG("%s\n", rbuf);  

	sprintf(userlist,"%s",to);
	userto = strtok(userlist,",");
	while(userto !=NULL){
		sprintf(buf, "RCPT TO: <%s>\r\n",userto);
		send(sockfd, buf, strlen(buf), 0);  
		memset(rbuf, 0, 1500);  
		recv(sockfd, rbuf, 1500, 0);
		userto = strtok(NULL,",");
		DEBUG_MSG("%s\n", rbuf);  
	}
	// DATA 准备开始发送邮件内容  
	sprintf(buf, "DATA\r\n");  
	send(sockfd, buf, strlen(buf), 0);  
	memset(rbuf, 0, 1500);  
	recv(sockfd, rbuf, 1500, 0);  
	DEBUG_MSG("%s\n", rbuf);  
	// 发送邮件内容，\r\n.\r\n内容结束标记  
	sprintf(buf, "%s\r\n.\r\n", body);  
	send(sockfd, buf, strlen(buf), 0);  
	memset(rbuf, 0, 1500);  
	recv(sockfd, rbuf, 1500, 0);  
	DEBUG_MSG("%s\n", rbuf);  
	// QUIT  
	sprintf(buf, "QUIT\r\n");  
	send(sockfd, buf, strlen(buf), 0);  
	memset(rbuf, 0, 1500);  
	recv(sockfd, rbuf, 1500, 0);  
	DEBUG_MSG("%s\n", rbuf);
	close(sockfd);  
	return TRUE;
}  


static char con628(char c6)  
{  
    char rtn = '\0';  
    if (c6 < 26)  
        rtn = c6 + 65;  
    else if (c6 < 52)  
        rtn = c6 + 71;  
    else if (c6 < 62)  
        rtn = c6 - 4;  
    else if (c6 == 62)  
        rtn = 43;  
    else  
        rtn = 47;  
  
    return rtn;  
}  

static void base64(char *dbuf, char *buf128, int len)  
{  
    struct data6 *ddd = NULL;  
    int i = 0;  
    char buf[256] = {0};  
    char *tmp = NULL;  
    char cc = '\0';  
    memset(buf, 0, 256);  
    strcpy(buf, buf128);  
    for(i = 1; i <= len/3; i++)  
    {  
        tmp = buf+(i-1)*3;  
        cc = tmp[2];  
        tmp[2] = tmp[0];  
        tmp[0] = cc;  
        ddd = (struct data6 *)tmp;  
        dbuf[(i-1)*4+0] = con628((unsigned int)ddd->d1);  
        dbuf[(i-1)*4+1] = con628((unsigned int)ddd->d2);  
        dbuf[(i-1)*4+2] = con628((unsigned int)ddd->d3);  
        dbuf[(i-1)*4+3] = con628((unsigned int)ddd->d4);  
    }  
    if(len%3 == 1)  
    {  
        tmp = buf+(i-1)*3;  
        cc = tmp[2];  
        tmp[2] = tmp[0];  
        tmp[0] = cc;  
        ddd = (struct data6 *)tmp;  
        dbuf[(i-1)*4+0] = con628((unsigned int)ddd->d1);  
        dbuf[(i-1)*4+1] = con628((unsigned int)ddd->d2);  
        dbuf[(i-1)*4+2] = '=';  
        dbuf[(i-1)*4+3] = '=';  
    }  
    if(len%3 == 2)  
    {  
        tmp = buf+(i-1)*3;  
        cc = tmp[2];  
        tmp[2] = tmp[0];  
        tmp[0] = cc;  
        ddd = (struct data6 *)tmp;  
        dbuf[(i-1)*4+0] = con628((unsigned int)ddd->d1);  
        dbuf[(i-1)*4+1] = con628((unsigned int)ddd->d2);  
        dbuf[(i-1)*4+2] = con628((unsigned int)ddd->d3);  
        dbuf[(i-1)*4+3] = '=';  
    }  
    return;  
} 

static int open_socket(struct sockaddr *addr)  
{  
	int sockfd = 0;
	struct timeval timeout = {35, 0};
	socklen_t len = sizeof(timeout);
	
	sockfd=socket(AF_INET, SOCK_STREAM, 0);  
	if(sockfd < 0)  
	{  
		DEBUG_MSG("Open sockfd(TCP) error!\n");  
		return -1;  
	}  
	if (setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &timeout, len) == -1)
	{
		perror("setsockopt error");
		return -1;
	}
	if(connect(sockfd, addr, sizeof(struct sockaddr)) < 0)  
	{  
		DEBUG_MSG("Connect sockfd(TCP) error!\n");
		close(sockfd);
		return -1;
	}  
	return sockfd;  
} 


BOOL GetIPByDomainName(const char *domainName,char *ip)
{
	struct hostent *hptr;
	char **pptr;
	char name[50] = {0};
	char str[20] = {0};
	strcpy(name,domainName);
	DEBUG_MSG("domainname:%s\n", name);
	if((hptr = gethostbyname(name)) == NULL)
	{
		DEBUG_MSG("gethostbyname error for host:%s\n", name);
		return FALSE;
	}
	switch(hptr->h_addrtype)
	{
		case AF_INET:
		case AF_INET6:
			pptr=hptr->h_addr_list;
			for(; *pptr!=NULL; pptr++){
				DEBUG_MSG("address:%s\n",inet_ntop(hptr->h_addrtype, *pptr, str, sizeof(str)));
			}
			strcpy(ip,inet_ntop(hptr->h_addrtype, hptr->h_addr, str, sizeof(str)));
			DEBUG_MSG("first address: %s\n",ip);
			return TRUE;
			break;
		default:
			DEBUG_MSG("unknown address type\n");
			break;
	}
	return FALSE;
}
