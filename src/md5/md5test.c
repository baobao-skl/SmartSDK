#include <stdio.h>  
#include "md5.h"  
  
int main(int argc, char *argv[])  
{  
    char buffer[33]={0};
	if(md5encode(argv[1],buffer)==0)
		printf("%s\n",buffer);
	else
		printf("fail\n");
    return 0;  
}  