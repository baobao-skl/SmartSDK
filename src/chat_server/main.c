#include <stdio.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <fcntl.h>
#include <linux/soundcard.h>
#include <sys/ioctl.h>

#define DEBUG
#ifdef DEBBUG
	#define DEBUG_MSG printf
#else
	#define DBUG_MSG(...)
#endif

#define SOUND_BITS 16

/* 1 for mono, 2 for stereo */
#define SOUND_CHANNELS 1

void *DoListen(void *arg);
int CreateUDPSocket(void);

static int sockfd = 0;
static int fd = 0;

int SoundOpen (int oflag) 
{
  int arg;
  int status;

  fd = open("/dev/dsp", oflag);
  if (fd < 0) {
    perror ("open of /dev/dsp failed");
    return -1;
  }

  /* Set PCM bits */
  arg = SOUND_BITS;
  status = ioctl(fd, SOUND_PCM_WRITE_BITS, &arg);
  if (status == -1)
    perror ("SOUND_PCM_WRITE_BITS ioctl failed");
  else if (arg != SOUND_BITS)
    perror ("unable to set sample size");

  /* Set channels (stereo, mono) */
  arg = SOUND_CHANNELS;
  status = ioctl(fd, SOUND_PCM_WRITE_CHANNELS, &arg);
  if (status == -1)
    perror ("SOUND_PCM_WRITE_CHANNELS ioctl failed");
  else if (arg != SOUND_CHANNELS)
    perror ("unable to set number of channels");

  /* 2 fragments, size = SAMPLES */
  arg = (2 << 16) + 9;
  printf ("arg = %x\n", arg);
  status = ioctl (fd, SNDCTL_DSP_SETFRAGMENT, &arg);
  if (status == -1)
    perror ("SNDCTL_DSP_SETFRAGMENT ioctl failed");

  return (fd);
}


void SoundPlay(char *stream,int len)
{
	write(fd,stream,len);
}

void SoundClose(void)
{
	close(fd);
}

int main(int argc, char **argv)
{
	SoundOpen(O_WRONLY | O_SYNC);
	pthread_t listenConnectThreadID;
	pthread_create(&listenConnectThreadID, NULL, DoListen, NULL);
	pthread_detach(listenConnectThreadID);
	pause();
	return 0;
}

int CreateUDPSocket(void)
{
	struct sockaddr_in servaddr;
	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
		
	if(sockfd < 0){
		return -1;
	}

	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family			=	 AF_INET;
	servaddr.sin_port 			= 	htons(9000);
	servaddr.sin_addr.s_addr		= 	htonl(INADDR_ANY);

	if(bind(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0){
		return -1;
	}
	return sockfd;
}

void * DoListen(void *arg)
{
		char buffer[1024] = {0};
		int len = 0;
		int addr_len = 0;;
		struct sockaddr_in client_addr;
		
		sockfd = CreateUDPSocket();
		addr_len = sizeof(struct sockaddr_in);
		
		while(1){
				memset(buffer, 0,sizeof(buffer));
				len = recvfrom(sockfd, buffer, sizeof(buffer), 0 , (struct sockaddr *)&client_addr, &addr_len);   
				printf("receive from %s : len = %d\n" , inet_ntoa( client_addr.sin_addr), len);
				SoundPlay(buffer,len);
		}
		return NULL;
}


