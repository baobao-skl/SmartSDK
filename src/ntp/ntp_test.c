#include <sys/socket.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/un.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <netdb.h>

#define NTP_PORT 		123
#define TIME_PORT 		37
#define NTP_SERVER_IP 	"210.72.145.44"
#define NTP_PORT_STR 	"123"
#define NTPV1 			"NTP/V1"
#define NTPV2 			"NTP/V2"
#define NTPV3 			"NTP/V3"
#define NTPV4			"NTP/V4"
#define TIME				"TIME/UDP"

#define NTP_PCK_LEN		48
#define LI				0
#define VN				3
#define MODE			3
#define STRATUM			0
#define POLL 				4
#define PREC				-6

#define JAN_1970		0x83aa7e80
#define NTPFRAC(x)		(4294 * (x) + ((1981 * (x)) >> 11))
#define USEC(x)			(((x) >> 12) -(((759* (x) >> 10) + 32768) >> 16))

typedef struct _ntp_time{
	unsigned int coarse;
	unsigned int fine;
}ntp_time;

struct ntp_packet{
	unsigned char leap_ver_mode;
	unsigned char startum;
	char poll;
	char precision;
	int root_delay;
	int root_dispersion;
	int reference_identifier;
	ntp_time reference_timestamp;
	ntp_time originage_timestamp;
	ntp_time receive_timestamp;
	ntp_time transmit_timestamp;
};

char protocal[32];

int construct_packet(char *packet)
{
	char version = 1;
	long tmp_wrd;
	int port;
	time_t timer;
	strcpy(protocal,NTPV3);

	if(!strcmp(protocal ,NTPV1) || !strcmp(protocal, NTPV2) || !strcmp(protocal, NTPV3) || !strcmp(protocal, NTPV4))
	{
		memset(packet, 0 , NTP_PCK_LEN);
		port = NTP_PORT;

		version = protocal[6] - 0x30;
		tmp_wrd = htonl((LI << 30) | (version << 27) | (MODE << 24) | (STRATUM << 16) | (POLL << 8) | (PREC & 0xff));
		memcpy(packet , &tmp_wrd , sizeof(tmp_wrd));

		tmp_wrd = htonl(1<<16);
		memcpy(&packet[4], &tmp_wrd , sizeof(tmp_wrd));
		memcpy(&packet[8], &tmp_wrd , sizeof(tmp_wrd));

		time(&timer);

		tmp_wrd = htonl(JAN_1970 + (long)timer);
		memcpy(&packet[40], &tmp_wrd , sizeof(tmp_wrd));

		tmp_wrd = htonl((long)NTPFRAC(timer));
		memcpy(&packet[44], &tmp_wrd, sizeof(tmp_wrd));
		return NTP_PCK_LEN;
	}
	else if(!strcmp(protocal, TIME))
	{
		port = TIME_PORT;
		memset(packet , 0 , 4);
		return 4;
	}
	return 0;
}

int get_ntp_time(int sk, struct addrinfo *addr, struct ntp_packet *ret_time)
{
	fd_set pending_data;
	struct timeval block_time;
	char data[NTP_PCK_LEN * 8] = {0};
	int packet_len, data_len = addr->ai_addrlen, count = 0, result , i, re;
	if(!(packet_len = construct_packet(data))){
		printf("construct packet error!\n");
		return 0;
	}

	if((result = sendto(sk,data,packet_len,0,addr->ai_addr,data_len)) < 0 ){
		printf("send packet error!\n");
		return -1;
	}

	FD_ZERO(&pending_data);
	FD_SET(sk , &pending_data);
	block_time.tv_sec = 10;
	block_time.tv_usec = 0;
	if(select(sk + 1, &pending_data,NULL,NULL,&block_time) > 0)
	{
		if( (count = recvfrom(sk,data , NTP_PCK_LEN * 8, 0, addr->ai_addr, &data_len)) < 0)
		{
			return 0;
		}

		if(protocal == TIME)
		{
			memcpy(&ret_time->transmit_timestamp, data , 4);
			return 1;
		}
		else if(count < NTP_PCK_LEN)
		{
			return 0;
		}
		ret_time->leap_ver_mode			=	ntohl(data[0]);
		ret_time->startum				=	ntohl(data[1]);
		ret_time->poll					=	ntohl(data[2]);
		ret_time->precision				=	ntohl(data[3]);
		ret_time->root_delay				=	ntohl(*(int *)&(data[4]));
		ret_time->root_dispersion			=	ntohl(*(int *)&(data[8]));
		ret_time->reference_identifier		=	ntohl(*(int *)&(data[12]));
		ret_time->receive_timestamp.coarse=	ntohl(*(int *)&(data[16]));
		ret_time->receive_timestamp.fine	=	ntohl(*(int *)&(data[20]));
		ret_time->originage_timestamp.coarse	=	ntohl(*(int *)&(data[24]));
		ret_time->originage_timestamp.fine		=	ntohl(*(int *)&(data[28]));
		ret_time->receive_timestamp.coarse		=	ntohl(*(int *)&(data[32]));
		ret_time->receive_timestamp.fine		=	ntohl(*(int *)&(data[36]));
		ret_time->transmit_timestamp.coarse	=	ntohl(*(int *)&(data[40]));
		ret_time->transmit_timestamp.fine		=	ntohl(*(int *)&(data[44]));
		return 1;
	}
	return 0;
}

int set_local_time(struct ntp_packet *pnew_time_packet)
{
	struct timeval tv;
	tv.tv_sec		=	pnew_time_packet->transmit_timestamp.coarse - JAN_1970;
	tv.tv_usec	=	USEC(pnew_time_packet->transmit_timestamp.fine);
	return settimeofday(&tv,NULL);
}

int main()
{
	int sockfd, rc;
	struct addrinfo hints, *res = NULL;
	struct ntp_packet	new_time_packet;

	memset(&hints, 0 , sizeof(hints));

	hints.ai_family		=	AF_UNSPEC;
	hints.ai_socktype	=	SOCK_DGRAM;
	hints.ai_protocol	=	IPPROTO_UDP;

	rc = getaddrinfo(NTP_SERVER_IP, NTP_PORT_STR, &hints, &res);
	if(rc != 0){
		printf("dns parse error!\n");
		return 1;
	}

	sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if(sockfd < 0){
		printf("create socket error!\n");
		return 1;
	}

	if(get_ntp_time(sockfd, res, &new_time_packet))
	{
		if(!set_local_time(&new_time_packet))
		{
			printf("ntp set time success!\n");
		}else{
			printf("ntp set time error!\n");
		}
	}else{
		printf("get ntp time error!\n");
	}
	close(sockfd);
	return 0;
}

