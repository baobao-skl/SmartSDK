/* comm.h - Communication input/output modules
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more
 * details (available in COPYING).
 */

#include "e-comm.h"
#include "comm.h"
#include <sys/types.h>
#include <sys/socket.h>

extern rtp_t rtp;
extern int open_soundcard (int oflag);

void *mod_cin (void *ptr)
{
  int rtpfd;                    /* RTP socket file descriptor */
  int rtcpfd;                   /* RTCP socket file descriptor */
  socklen_t addr_len;                 /* Data size, Bytes received */
  struct sockaddr_in rtps;      /* RTP socket */
  struct sockaddr_in rtcps;     /* RTCP socket */
  struct sockaddr_in remote;    /* Remote address information */
  vstr_t recv_data;             /* Received data */
  rtp_packet_t *packet;         /* Parsed RTP packet */

  fd_set readset;
  int fdmax;

  fprintf(stderr, "+ Communication input module loaded.\n");

  vstr_init (&recv_data, RTP_MTU_SIZE);

  if ((rtpfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("Error opening socket");
    exit(1);
  }

  if ((rtcpfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("Error opening socket");
    exit(1);
  }

  rtps.sin_family = AF_INET;			/* Internet protocol */
  rtps.sin_port = htons(RTP_PORT);		/* Port */
  rtps.sin_addr.s_addr = htonl(INADDR_ANY);	/* rtps address */
  memset(&(rtps.sin_zero), '\0', 8);		/* Zero the rest */

  if (bind(rtpfd, (struct sockaddr *) &rtps, sizeof(struct sockaddr)) < 0) {
    perror("Error binding socket");
    exit(1);
  }

  rtcps.sin_family = AF_INET;			/* Internet protocol */
  rtcps.sin_port = htons(RTCP_PORT);		/* Port */
  rtcps.sin_addr.s_addr = htonl(INADDR_ANY);	/* rtcps address */
  memset(&(rtcps.sin_zero), '\0', 8);		/* Zero the rest */

  if (bind(rtcpfd, (struct sockaddr *) &rtcps, sizeof(struct sockaddr)) < 0) {
    perror("Error binding socket");
    exit(1);
  }

  fprintf(stderr, "+ RTP Listening at %s:%d...\n",
        inet_ntoa(rtps.sin_addr), ntohs(rtps.sin_port));

  fprintf(stderr, "+ RTCP Listening at %s:%d...\n",
        inet_ntoa(rtcps.sin_addr), ntohs(rtcps.sin_port));

  addr_len = sizeof(struct sockaddr);

  while(1)
  {
    FD_ZERO (&readset);
    FD_SET (rtpfd, &readset);
    FD_SET (rtcpfd, &readset);
    fdmax = (rtpfd < rtcpfd) ? rtcpfd : rtpfd;

    select (fdmax + 1, &readset, NULL, NULL, NULL);

    if (FD_ISSET(rtpfd, &readset)) {

      
      /* Receive data from network */
      recv_data.size = recvfrom (rtpfd, recv_data.data, RTP_MTU_SIZE, 0, (struct sockaddr *)&remote, &addr_len);
      printf("receive data, size = %d!!!!!!!!!!!!!!!!!\n", recv_data.size);
      vstr_adv_tail (&recv_data, recv_data.size);
      if (recv_data.size < 0) {
        perror("Error receiving data from socket");
        exit(1);
      }

      /* Write to buffer */
      packet = rtp_recv (&rtp, &recv_data);
      if (packet != NULL && rtp_enqueue (&rtp, packet) == -1)
        rtp_packet_free (packet);

      vstr_flush (&recv_data);
    }

    if (FD_ISSET(rtcpfd, &readset)) {

      /* Receive data from network */
      recv_data.size = recvfrom (rtcpfd, recv_data.data, RTP_MTU_SIZE, 0, (struct sockaddr *)&remote, &addr_len);
      vstr_adv_tail (&recv_data, recv_data.size);
      if (recv_data.size < 0) {
        perror("Error receiving data from socket");
        exit(1);
      }

      rtcp_recv (&rtp, &recv_data);

      vstr_flush (&recv_data);
    }

  }
}

void *mod_cout (void *ptr)
{
  int sockfd;                   /* Socket file descriptor */
  int sent_bytes;               /* Bytes sent */
  struct hostent *hinfo;        /* Host information */
  struct sockaddr_in remote;    /* Remote address information */
  vstr_t vstr;                  /* Data to send */

  int soundfd;                  /* Sound device file descriptor */
  struct adpcm_state state;     /* ADPCM state, see adpcm.c */

  short inbuf[SAMPLES];         /* Input buffer to get sound samples */
  char outbuf[SAMPLES/2];       /* Output buffer to receive ADPCM code */

  rtp_param_t param;            /* RTP parameters */
  vstr_t ctrl;

  char *hostname = (char *) ptr;
  int i;

  DEBUG_MSG("hostname:--%s--\n", hostname);
  fprintf(stderr, "+ Communication output module loaded.\n");
  fprintf(stderr, "+ Sound input module loaded.\n");

  vstr_init (&vstr, RTP_MTU_SIZE);// init vstr size, data part
  vstr_init (&ctrl, RTP_MTU_SIZE);

  /* Initialize ADPCM encoder */
  state.valprev = 0;
  state.index = 0; 

  /* Get target parameters */
  if ((hinfo = gethostbyname(hostname)) == (struct hostent *) NULL) {
    perror("Error resolving host name");
    exit(1);
  }

  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("Error opening socket");
    exit(1);
  }

  remote.sin_family = AF_INET;                  /* Internet protocol */
  remote.sin_addr = *((struct in_addr *) hinfo -> h_addr); /* Address */
  memset(&(remote.sin_zero), '\0', 8);          /* Zero the rest */


  /* Open sound device to read */
  soundfd = open_soundcard (O_RDONLY);

  param.flags = 0;
  param.pt = 3;
  param.len = SAMPLES / 2;
  param.payload = outbuf;

  for (;;) {
    DEBUG_MSG("%s test\n",__FUNCTION__);
    vstr_flush (&ctrl);
    rtcp_send_ctrl (&rtp, &ctrl);
    rtcp_append_sdes (&rtp, &ctrl, SDES_CNAME | SDES_TOOL);

    remote.sin_port = htons(RTCP_PORT);         /* Port */
    sent_bytes = sendto(sockfd, ctrl.head, ctrl.size, 0,
                (struct sockaddr *) &remote, sizeof(struct sockaddr));

    //This will send data to speficy host directly
    for (i = 0; i < 100; i++)
    {
      DEBUG_MSG("%s read data start\n",__FUNCTION__);
      /* Read from sound device */
      read (soundfd, inbuf, sizeof(inbuf));
	DEBUG_MSG("%s read data over\n",__FUNCTION__);
      /* Encodes data */
      adpcm_coder (inbuf, outbuf, SAMPLES, &state);
      /* Create RTP packet */
      vstr_flush (&vstr);
      rtp_send (&rtp, &param, &vstr);

      /* Send to network */
      remote.sin_port = htons(RTP_PORT);        /* Port */
      sent_bytes = sendto(sockfd, vstr.head, vstr.size, 0,
                  (struct sockaddr *) &remote, sizeof(struct sockaddr));
    }
  }
}
