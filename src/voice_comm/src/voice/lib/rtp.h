/* rtp.h - Real Time Protocol                                             */
/*                                                                        */
/* This program is free software; you can redistribute it and/or modify   */
/* it under the terms of the GNU General Public License as published by   */
/* the Free Software Foundation; either version 2, or (at your option)    */
/* any later version.                                                     */
/*                                                                        */
/* This program is distributed in the hope that it will be useful,        */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of         */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          */
/* GNU General Public License for more details.                           */
/*                                                                        */
/* You should have received a copy of the GNU General Public License      */
/* along with this program; if not, write to the Free Software            */
/* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA              */
/* 02111-1307, USA.                                                       */

#ifndef RTP_H
#define RTP_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <netinet/in.h>

#include "config.h"
#include "alloc.h"
#include "seterror.h"
#include "rand32.h"
#include "ohtbl.h"
#include "vstr.h"
#include "phtbl.h"

/* RTP build flags */
#define RTP_SENDER              0x01
#define RTP_MIXER               0x02

/* RTP defaults */
#define RTP_VERSION             2
#define RTP_MTU_SIZE            1500
#define RTP_MAX_QUEUE           32
#define RTP_SOLVE_TIME          1024
#define RTP_SDES_MAX            255
#define RTP_MAX_FAIL_COUNT      5
#define RTP_HEADER_SIZE         3*sizeof(uint32_t)
#define RTCP_HEADER_SIZE        2*sizeof(uint32_t)
#define RTCP_SR_SIZE            5*sizeof(uint32_t)
#define RTCP_RR_SIZE            6*sizeof(uint32_t)
#define RTCP_SDES_HEADER_SIZE   2*sizeof(uint16_t)
#define RTCP_SDES_ITEM_SIZE     2*sizeof(uint8_t)

/* Total number of sources (hash = 103, tolerance = 50) */
#define RTP_HASH_SIZE           103
#define RTP_HASH_TOLERANCE      50

/* RTP flags */
#define FLAG_MARKER             0x01
#define FLAG_PADDING            0x02
#define FLAG_EXTENSION          0x04

/* RTCP Reports */
#define RTCP_SR                 200
#define RTCP_RR                 201
#define RTCP_SDES               202
#define RTCP_BYE                203
#define RTCP_APP                204

/* Check for valid RTCP types */
#define RTCP_TYPE_MIN           72      /* 200 & 0x7F */
#define RTCP_TYPE_MAX           76      /* 200 & 0x7F */

/* RTCP Source Description Reports */
#define RTCP_SDES_END           0
#define RTCP_SDES_CNAME         1
#define RTCP_SDES_NAME          2
#define RTCP_SDES_EMAIL         3
#define RTCP_SDES_PHONE         4
#define RTCP_SDES_LOC           5
#define RTCP_SDES_TOOL          6
#define RTCP_SDES_NOTE          7
#define RTCP_SDES_LAST          8

/* RTCP indexes */
#define CNAME                   0
#define NAME                    1
#define EMAIL                   2
#define PHONE                   3
#define LOC                     4
#define TOOL                    5
#define NOTE                    6

/* rtcp_sdes interface: flags */
#define SDES_CNAME              0x0001
#define SDES_NAME               0x0002
#define SDES_EMAIL              0x0004
#define SDES_PHONE              0x0008
#define SDES_LOC                0x0010
#define SDES_TOOL               0x0020
#define SDES_NOTE               0x0040
#define SDES_ALL                0x007F

/* correct memory allocation for direct mapping */
#define RTP_PACKET_IDX    2*sizeof(uint32_t)+sizeof(int)+sizeof(void *)
#define RTP_PACKET_SIZE   RTP_PACKET_IDX+RTP_MTU_SIZE
#define RTCP_PACKET_IDX   31*(7*sizeof(rtcp_sdes_item_t *)+sizeof(rtcp_sdes_t *))
#define RTCP_PACKET_SIZE  RTCP_PACKET_IDX+RTP_MTU_SIZE
#define RTCP_BYE_IDX      sizeof(rtcp_bye_t)-sizeof(uint8_t)-sizeof(uint8_t *)
#define RTCP_BYE_SIZE     RTCP_BYE_IDX+RTP_MTU_SIZE
#define RTCP_APP_IDX      sizeof(rtcp_app_t)-sizeof(char *)
#define RTCP_APP_SIZE     RTCP_APP_IDX+RTP_MTU_SIZE

/* RTP packet */
typedef struct {
  uint16_t xdef;                /* defined by profile                     */
  uint16_t xlen;                /* lenght (32-bit words minus one)        */
  uint32_t *xhdr;               /* extra fields                           */
  int pay_len;                  /* Payload lenght                         */
  void *payload;                /* RTP payload                            */
#ifdef WORDS_BIGENDIAN
  unsigned int ver: 2;          /* RTP version                            */
  unsigned int p: 1;            /* padding flag                           */
  unsigned int x: 1;            /* header extension flag                  */
  unsigned int cc: 4;           /* CSRC count                             */
  unsigned int m: 1;            /* marker bit                             */
  unsigned int pt: 7;           /* payload type                           */
#else
  unsigned int cc: 4;           /* CSRC count                             */
  unsigned int x: 1;            /* header extension flag                  */
  unsigned int p: 1;            /* padding flag                           */
  unsigned int ver: 2;          /* RTP version                            */
  unsigned int pt: 7;           /* payload type                           */
  unsigned int m: 1;            /* marker bit                             */
#endif
  uint16_t seq;                 /* sequence number                        */
  uint32_t ts;                  /* timestamp                              */
  uint32_t ssrc;                /* synchronization source                 */
  uint32_t csrc[1];             /* optional CSRC list                     */
} rtp_packet_t;                 /* RTP packet                             */

/* RTCP report block */
typedef struct {
  uint32_t ssrc;                /* SSRC identifier of the source          */
#ifdef WORDS_BIGENDIAN
  uint8_t frac;                 /* fraction of packets lost               */
  unsigned int lost: 24;        /* packets lost                           */
#else
  unsigned int lost: 24;        /* packets lost                           */
  uint8_t frac;                 /* fraction of packets lost               */
#endif
  uint32_t last_seq;            /* extended last sequence number received */
  uint32_t jitter;              /* interarrival jitter                    */
  uint32_t lsr;                 /* last sender report                     */
  uint32_t dlsr;                /* delay since last sender report         */
} rtcp_report_t;

/* RTCP SR */
typedef struct {
  uint32_t ssrc;                /* sender generating this report          */
  uint32_t ntp_sec;             /* NTP timestamp (seconds)                */
  uint32_t ntp_frac;            /* NTP timestamp (fraction of second)     */
  uint32_t rtp_ts;              /* RTP timestamp                          */
  uint32_t pcount;              /* sender's packet count                  */
  uint32_t ocount;              /* sender's octet count                   */
  rtcp_report_t rr[1];          /* reception report block list            */
} rtcp_sr_t;

/* RTCP RR */
typedef struct {
  uint32_t ssrc;                /* receiver generating this report        */
  rtcp_report_t rr[1];          /* reception report block list            */
} rtcp_rr_t;

/* RTCP SDES (item) */
typedef struct {
  uint8_t type;                 /* type of item                           */
  uint8_t length;               /* length of item (in octets)             */
  char data[1];                 /* text, not null-terminated              */
} rtcp_sdes_item_t;

/* RTCP SDES */
typedef struct {
  uint32_t ssrc;                /* SSRC/CSRC identifier                   */
  rtcp_sdes_item_t item[1];     /* SDES items                             */
} rtcp_sdes_t;

/* RTCP BYE */
typedef struct {
  uint8_t rlen;                 /* reason length                          */
  char *reason;                 /* reason for leaving                     */
#ifdef WORDS_BIGENDIAN
  unsigned int ver: 2;          /* RTP version                            */
  unsigned int p: 1;            /* padding flag                           */
  unsigned int sc: 5;           /* source count                           */
#else
  unsigned int sc: 5;           /* source count                           */
  unsigned int p: 1;            /* padding flag                           */
  unsigned int ver: 2;          /* RTP version                            */
#endif
  uint8_t pt;                   /* packet type                            */
  uint16_t len;                 /* length of rtcp packet                  */
  uint32_t ssrc[1];             /* SSRC/CSRC identifier                   */
} rtcp_bye_t;

/* RTCP APP */
typedef struct {
  char *data;                   /* application-dependent data             */
#ifdef WORDS_BIGENDIAN
  unsigned int ver: 2;          /* RTP version                            */
  unsigned int p: 1;            /* padding flag                           */
  unsigned int subtype: 5;      /* subtype                                */
#else
  unsigned int subtype: 5;      /* subtype                                */
  unsigned int p: 1;            /* padding flag                           */
  unsigned int ver: 2;          /* RTP version                            */
#endif
  uint8_t pt;                   /* packet type                            */
  uint16_t len;                 /* length of rtcp packet                  */
  uint32_t ssrc;                /* SSRC/CSRC identifier                   */
  char name[4];                 /* name of APP packet                     */
} rtcp_app_t;

/* RTCP packet */
typedef struct {
#ifdef WORDS_BIGENDIAN
  unsigned int ver: 2;          /* RTP version                            */
  unsigned int p: 1;            /* padding flag                           */
  unsigned int count: 5;        /* depends of packet type                 */
#else
  unsigned int count: 5;        /* depends of packet type                 */
  unsigned int p: 1;            /* padding flag                           */
  unsigned int ver: 2;          /* RTP version                            */
#endif
  uint8_t pt;                   /* packet type                            */
  uint16_t len;                 /* length of rtcp packet                  */
  union {
    uint32_t ssrc;              /* SSRC id                                */
    rtcp_sr_t sr;               /* sender report                          */
    rtcp_rr_t rr;               /* receiver report                        */
    rtcp_sdes_t sdes;           /* source description                     */
  } r;                          /* rtcp report                            */
} rtcp_packet_t;

/* RTP session */
typedef struct {
  pthread_mutex_t guard;        /* session guard                          */
  uint32_t ssrc;                /* synchronization source                 */
  unsigned int cc: 4;           /* CSRC count                             */
  uint32_t csrc[15];            /* CSRC list                              */
  uint16_t seq;                 /* sequence number                        */
  uint16_t cycles;              /* sequence number cycles                 */
  uint32_t pcount;              /* packet count                           */
  uint32_t ocount;              /* octet count                            */
  char cname[RTP_SDES_MAX];     /* canonical name                         */
  char name[RTP_SDES_MAX];      /* user name                              */
  char email[RTP_SDES_MAX];     /* electronic mail                        */
  char phone[RTP_SDES_MAX];     /* phone number                           */
  char loc[RTP_SDES_MAX];       /* user location                          */
  char tool[RTP_SDES_MAX];      /* application/tool name                  */
  char note[RTP_SDES_MAX];      /* notice/status                          */
  ohtbl_t members;              /* table of RTP sources                   */
  int flags;                    /* RTP build flags                        */
} rtp_t;                        /* RTP sender session                     */

/* RTP source */
typedef struct {
  pthread_mutex_t guard;        /* session guard                          */
  uint32_t ssrc;                /* synchronization source                 */
  uint16_t max_seq;             /* sequence number                        */
  uint16_t cycles;              /* sequence number cycles                 */
  uint32_t received;            /* total packets received                 */
  unsigned int lost: 24;        /* cumulative number of packets lost      */
  uint8_t frac;                 /* fraction of packets lost               */
  uint32_t last_seq;            /* extended last sequence number received */
  long transit;                 /* relative transit time (prev packet)    */
  long jitter;                  /* estimated jitter                       */
  long recv_jitter;             /* estimated reception jitter             */
  uint32_t rtt;                 /* round trip time                        */
  uint32_t lsr;                 /* last sender report                     */
  uint32_t dlsr;                /* delay since last sender report         */
  uint16_t min_seq;             /* minimal sequence number received       */
  int fail_count;               /* failed packet queries                  */
  phtbl_t pqueue;               /* queue of received packets              */
  char cname[RTP_SDES_MAX];     /* canonical name                         */
  char name[RTP_SDES_MAX];      /* user name                              */
  char email[RTP_SDES_MAX];     /* electronic mail                        */
  char phone[RTP_SDES_MAX];     /* phone number                           */
  char loc[RTP_SDES_MAX];       /* user location                          */
  char tool[RTP_SDES_MAX];      /* application/tool name                  */
  char note[RTP_SDES_MAX];      /* notice/status                          */
} rtp_source_t;                 /* RTP source                             */

/* RTP build parameters */
typedef struct {
  uint8_t flags;                /* RTP flags                              */
  uint16_t xdef;                /* header extension defined by profile    */
  uint16_t xlen;                /* header extension lenght                */
  uint32_t *xhdr;               /* header extension extra fields          */
  unsigned int pt: 7;           /* payload type                           */
  int len;                      /* payload lenght                         */
  char *payload;                /* payload                                */
} rtp_param_t;

/* Initializes a RTP 'session'.                                           */
void rtp_init (rtp_t *s, int flags);

/* Destroy RTP session                                                    */
void rtp_destroy (rtp_t *s);

/* Adds CSRC to list. 'param' is the list of RTP parameters, and 'csrc'   */
/* is the CSRC id of contributing source. Returns 0 on fail, 1 on         */
/* success. On error, the error message will be send to 'stderr'. The     */
/* CSRC entries are saved in network byte order.                          */
int rtp_add_csrc (rtp_t *s, uint32_t csrc);

/* Deletes CSRC from list. 'param' is the list of RTP parameters, and     */
/* 'csrc' s the contributing source to be removed. Returns 0 on fail, 1   */
/* on success. On error, the error message will be send to 'stderr'. The  */
/* CSRC entries are saved in network byte order.                          */
int rtp_del_csrc (rtp_t *s, uint32_t csrc);

/* Create a new RTP packet. 'param' is the struct containing all RTP      */
/* parameters, and 'session' is the RTP sender session.                   */
void rtp_send (rtp_t *s, rtp_param_t *param, vstr_t *p);

/* Free a received 'rtp_packet'.                                          */
void rtp_packet_free (rtp_packet_t *rtp_packet);

/* Parses received network 'packet' and checks for validity. Returns      */
/* pointer to new parsed RTP packet.                                      */
rtp_packet_t *rtp_recv (rtp_t *s, vstr_t *packet);

/* Creates a RTCP Sender Report + Receiver Reports */
void rtcp_send_ctrl (rtp_t *s, vstr_t *vstr);

/* Appends a RTCP Source Description report */
void rtcp_append_sdes (rtp_t *s, vstr_t *vstr, int flags);

/* Creates a RTCP Goodbye report */
vstr_t *rtcp_bye (uint32_t *ssrc_list, int list_len, char *reason, int len);

/* Creates a RTCP Application-Defined report */
/* packet_t *rtcp_app (void); */

/* Parses received RTCP packet, checks for validity and updates RTP session */
int rtcp_recv (rtp_t *s, vstr_t *packet);

/* Adds RTP packet to member packet queue */
int rtp_enqueue (rtp_t *s, rtp_packet_t *rtp_packet);

/* Get next RTP packet from packet queue */
rtp_packet_t *rtp_dequeue (rtp_t *s, uint32_t ssrc);

#endif /* RTP_H */
