#include "mac_helper.h"
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <string.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <netinet/in.h> 
#include <net/if.h> 
#include <unistd.h>

int getLocalMac(char *dev_name,char *mac)
{
	int sock_mac;
	struct ifreq ifr_mac;
	
	if(mac == NULL){
		return -1;
	}

	sock_mac = socket(AF_INET, SOCK_STREAM, 0);
	if (sock_mac == -1)
	{
		return -1;
	}

	memset(&ifr_mac, 0, sizeof(ifr_mac));
	strncpy(ifr_mac.ifr_name, dev_name, sizeof(ifr_mac.ifr_name) - 1);

	if ((ioctl(sock_mac, SIOCGIFHWADDR, &ifr_mac)) < 0)
	{
		close(sock_mac);
		return -1;
	}

	sprintf(mac, "%02X:%02X:%02X:%02X:%02X:%02X",
		(unsigned char)ifr_mac.ifr_hwaddr.sa_data[0],
		(unsigned char)ifr_mac.ifr_hwaddr.sa_data[1],
		(unsigned char)ifr_mac.ifr_hwaddr.sa_data[2],
		(unsigned char)ifr_mac.ifr_hwaddr.sa_data[3],
		(unsigned char)ifr_mac.ifr_hwaddr.sa_data[4],
		(unsigned char)ifr_mac.ifr_hwaddr.sa_data[5]
	);
	close(sock_mac);
	return 0;
}


