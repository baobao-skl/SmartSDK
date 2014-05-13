#include <stdio.h>
#include <fcntl.h>
#include <error.h>
int main(void)
{
	int fd = 0; 
	unsigned char temp[5] = {0,0};
	if((fd=open("/dev/dht11",0))<0)
	{
		perror("open dht11:");
		return;
	}
	while(1)
	{
		if(read(fd,(char *)temp,5)>0)	
			printf("wendu: %d.%dC shidu: %d.%d%%RH checksum = %d\n", temp[2],temp[3],temp[0],temp[1],temp[4]);
		sleep(1);
	}
	close(fd);
	return 0;
}
