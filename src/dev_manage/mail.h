#ifndef __MAIL_H__
#define __MAIL_H__
#include "dev.h"

struct data6  
{  
    unsigned int d4:6;  
    unsigned int d3:6;  
    unsigned int d2:6;  
    unsigned int d1:6;  
};  

typedef struct{
   char *name;
   int port;
   char *username;
   char *password;
}MAIL_SERVER;

typedef enum{
	SERVER_QQ,
	SERVER_126,
	SERVER_SOHU
}MAIL_SERVER_ITEM;

BOOL SendMail(char *to, char *body,MAIL_SERVER_ITEM server_type);
BOOL GetIPByDomainName(const char *domainName,char *ip);

#endif

