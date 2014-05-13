/* rtp.c - Real Time Protocol                                             */
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

#include "rtp.h"
#include "../e-comm.h"

/* used in NTP time calculation */
#define SECS_BETWEEN_1900_1970 2208988800u

/* tool identification */
char ecomm_id[] = "e-Comm (C) ComunIP";

/* Functions for double hashing */
unsigned int _rtp_members_h1 (const void *key) {
  return (((rtp_source_t *) key) -> ssrc % RTP_HASH_SIZE);
}

unsigned int _rtp_members_h2 (const void *key) {
  return (1 + ((rtp_source_t *) key) -> ssrc % (RTP_HASH_SIZE - 2));
}

/* Match function */
int _rtp_members_match (const void *key1, const void *key2) {
  return (((rtp_source_t *) key1) -> ssrc == ((rtp_source_t *) key2) -> ssrc);
}

/* Initializes a RTP session */
void rtp_init (rtp_t *s, int flags) {

  uint8_t *username = getenv("LOGNAME");
  uint8_t *hostname = getenv("HOSTNAME");

  DEBUG_MSG("%s username:--%s--\n",__FUNCTION__,username);
  DEBUG_MSG("%s hostname:--%s--\n",__FUNCTION__,hostname);

  pthread_mutex_init (&s -> guard, NULL);

  /* set RTP build flags */
  s -> flags = flags;

  s -> seq = (uint16_t) rand32();               /* initialize sequencing  */
  s -> cycles = 0;
  s -> ssrc = (uint32_t) rand32();              /* get random SSRC id     */
  s -> pcount = 0;
  s -> ocount = 0;
  s -> cc = 0;

  DEBUG_MSG("%s s -> seq:--%d--\n",__FUNCTION__,s -> seq);
  DEBUG_MSG("%s s -> ssrc:--%d--\n",__FUNCTION__,s -> ssrc);

  memset (s -> cname, '\0', RTP_SDES_MAX);
  memset (s -> name, '\0', RTP_SDES_MAX);
  memset (s -> email, '\0', RTP_SDES_MAX);
  memset (s -> phone, '\0', RTP_SDES_MAX);
  memset (s -> loc, '\0', RTP_SDES_MAX);
  memset (s -> tool, '\0', RTP_SDES_MAX);
  memset (s -> note, '\0', RTP_SDES_MAX);
  sprintf(s -> cname, "%s@%s", username, hostname);
  sprintf(s -> tool, "%s", ecomm_id);

  //this function mainly do init s -> members
  //
  ohtbl_init (&s -> members, RTP_HASH_SIZE, _rtp_members_h1,
        _rtp_members_h2, _rtp_members_match, NULL,
        RTP_HASH_TOLERANCE / 2);

  return;
}

/* Destroy RTP session */
void rtp_destroy (rtp_t *s) {

  rtp_source_t *member;

  pthread_mutex_lock (&s -> guard);

  ohtbl_flush (&s -> members);
  while ((member = (rtp_source_t *) ohtbl_traverse(&s -> members)) != NULL) {
    phtbl_destroy (&member -> pqueue);
    pthread_mutex_destroy (&member -> guard);
  }
  ohtbl_destroy (&s -> members);

  pthread_mutex_destroy (&s -> guard);

  return;
}

/* Add new csrc to RTP parameters,
   returns 0 on fail, 1 on success */
int rtp_add_csrc (rtp_t *s, uint32_t csrc) {

  int idx;              /* index */

  /* put csrc in network byte order */
  csrc = htonl (csrc);

  pthread_mutex_lock (&s -> guard);

  /* checks for maximal csrc number */
  if (s -> cc >= 15) {
    pthread_mutex_unlock (&s -> guard);
    return (0);
  }

  /* check if new csrc doesn't exist in csrc list */ 
  for (idx = 0; idx < s -> cc; idx++) {
    if (s -> csrc[idx] == csrc) {
      pthread_mutex_unlock (&s -> guard);
      return (0);
    }
  }

  /* just append new csrc to list */
  s -> csrc[s -> cc++] = csrc;

  pthread_mutex_unlock (&s -> guard);

  return (1);
}

/* Remove a csrc from list,
   returns 0 on fail, 1 on success */
int rtp_del_csrc (rtp_t *s, uint32_t csrc) {

  int cur, prev;        /* current and previous index */

  /* put csrc in network byte order */
  csrc = htonl (csrc);

  pthread_mutex_lock (&s -> guard);

  /* tries to remove element */
  for (cur = 0, prev = 0; prev < s -> cc; cur++, prev++) {
    if (s -> csrc[cur] == csrc)
      cur++;
    s -> csrc[prev] = s -> csrc[cur];
  }

  /* checks for empty csrc list */
  if (s -> cc > 0 && prev != cur) {
    s -> cc--;
    pthread_mutex_unlock (&s -> guard);
    return (1);
  }

  pthread_mutex_unlock (&s -> guard);
  return (0);
}

int rtp_update_sr (rtp_source_t *member, vstr_t *p) {

  rtcp_sr_t *sr = (rtcp_sr_t *) p -> head;

  member -> lsr = sr -> rtp_ts;

  return (0);
}

int rtp_update_rr (rtp_source_t *member, vstr_t *p) {

  rtcp_report_t *rr = (rtcp_report_t *) p -> head;
  struct timeval ts;

  gettimeofday (&ts, NULL);

  member -> rtt = (ts.tv_usec + ts.tv_sec * 1000000) - (rr -> lsr + rr -> dlsr);
  member -> jitter = rr -> jitter;

  return (0);
}

void rtp_update_sdes_item (char *data, int size, vstr_t *p, int *flag, int item) {

  if (!(*flag & item)) {
    *flag |= item;
    vstr_get_head (p, data, size);
  }
  else
    vstr_adv_head (p, size);

  return;
}

int rtp_update_sdes (rtp_source_t *member, vstr_t *p) {

  int flag = 0;
  rtcp_sdes_item_t *item = (rtcp_sdes_item_t *) p -> head;
  char *c;
  int i = 0;

  while (item -> type != RTCP_SDES_END) {
    vstr_adv_head (p, sizeof(uint16_t));

    switch (item -> type) {
      case RTCP_SDES_CNAME:
        rtp_update_sdes_item (member -> cname, item -> length, p, &flag, SDES_CNAME);
        break;
      case RTCP_SDES_NAME:
        rtp_update_sdes_item (member -> name, item -> length, p, &flag, SDES_NAME);
        break;
      case RTCP_SDES_EMAIL:
        rtp_update_sdes_item (member -> email, item -> length, p, &flag, SDES_EMAIL);
        break;
      case RTCP_SDES_PHONE:
        rtp_update_sdes_item (member -> phone, item -> length, p, &flag, SDES_PHONE);
        break;
      case RTCP_SDES_LOC:
        rtp_update_sdes_item (member -> loc, item -> length, p, &flag, SDES_LOC);
        break;
      case RTCP_SDES_TOOL:
        rtp_update_sdes_item (member -> tool, item -> length, p, &flag, SDES_TOOL);
        break;
      case RTCP_SDES_NOTE:
        rtp_update_sdes_item (member -> note, item -> length, p, &flag, SDES_NOTE);
        break;
    }

    item = (rtcp_sdes_item_t *) p -> head;
  }

  /* discard trailling null bytes */
  c = (char *) p -> head;
  while (i++ < sizeof(uint32_t) && *c == '\0') {
    vstr_adv_head (p, sizeof(uint8_t));
    c = (char *) p -> head;
  }

  return (0);
}

void rtp_discard_sdes (vstr_t *p) {

  rtcp_sdes_item_t *item = (rtcp_sdes_item_t *) p -> head;
  char *c;
  int i = 0;

  while (item -> type != RTCP_SDES_END) {
    vstr_adv_head (p, sizeof(uint16_t) + item -> length);
    item = (rtcp_sdes_item_t *) p -> head;
  }

  /* discard trailling null bytes */
  c = (char *) p -> head;
  while (i++ < sizeof(uint32_t) && *c == '\0') {
    vstr_adv_head (p, sizeof(uint8_t));
    c = (char *) p -> head;
  }

  return;
}

/* Create a new RTP packet */
void rtp_send (rtp_t *s, rtp_param_t *param, vstr_t *p) {

  struct timeval ts;
  rtp_packet_t *rtp_packet = (rtp_packet_t *) p -> data;

  /* initialize RTP packet */
  vstr_set (p, RTP_PACKET_IDX, RTP_PACKET_IDX);

  /* get timestamp */
  gettimeofday (&ts, NULL);

  /* build the header in network byte order */
  rtp_packet -> ver = RTP_VERSION;
  rtp_packet -> p = (param -> flags & FLAG_PADDING) ? 1 : 0;
  rtp_packet -> x = (param -> flags & FLAG_EXTENSION) ? 1 : 0;
  rtp_packet -> m = (param -> flags & FLAG_MARKER) ? 1 : 0;
  rtp_packet -> pt = param -> pt;
  rtp_packet -> ssrc = htonl(s -> ssrc);
  rtp_packet -> ts = htonl(ts.tv_usec + ts.tv_sec * 1000000);
  vstr_adv_tail (p, RTP_HEADER_SIZE);

  pthread_mutex_lock (&s -> guard);

  rtp_packet -> seq = htons(s -> seq);
  rtp_packet -> cc = s -> cc;

  /* update sequence number and check new cycle */
  if (++s -> seq == 0)
    s -> cycles++;

  /* update packet and octet counters */
  s -> pcount++;
  s -> ocount += p -> size;

  /* checks for optional CSRC list */
  if (s -> cc > 0)
    vstr_put_tail (p, s -> csrc, s -> cc * sizeof(uint32_t));

  pthread_mutex_unlock (&s -> guard);

  /* checks for optional header extension */
  if (rtp_packet -> x != 0) {

    /* add optional header extension */
    rtp_packet -> xdef = htons (param -> xdef);
    rtp_packet -> xlen = htons (param -> xlen);
    vstr_put_tail (p, &(rtp_packet -> xdef), 2 * sizeof(uint16_t));

    if (rtp_packet -> xlen != 0) {
      rtp_packet -> xhdr = (uint32_t *) p -> tail;
      vstr_put_tail (p, param -> xhdr, param -> xlen * sizeof(uint32_t));
    }
  }

  /* appends payload to packet */
  rtp_packet -> payload = p -> tail;
  vstr_put_tail (p, param -> payload, param -> len);

  return;
}

/* Free a received RTP packet */
void rtp_packet_free (rtp_packet_t *rtp_packet) {

  free (rtp_packet);

  return;
}

/* Updates jitter based on received packet */
void rtp_update_jitter (rtp_source_t *r, uint32_t pkt_ts) {

  long d;
  long transit;
  unsigned long arrival;
  struct timeval ts;

  /* interarrival jitter statistics */
  gettimeofday (&ts, NULL);
  arrival = ts.tv_usec + ts.tv_sec * 1000000;
  transit = arrival - pkt_ts;

  if (r -> transit == 0)
    r -> transit = transit;
  else {
    d = transit - r -> transit;
    r -> transit = transit;

    /* absolute value */
    if (d < 0)
      d = -d;

    r -> jitter += (1.0 / 16.0) * ((double) d - r -> jitter);
  }

  return;
}

/* Functions for packet queue */
unsigned int _rtp_pqueue_hash (const void *key) {
  return ((unsigned int)((rtp_packet_t *) key) -> seq);
}

int _rtp_pqueue_match (const void *key1, const void *key2) {
  return (((rtp_packet_t *) key1) -> seq == ((rtp_packet_t *) key2) -> seq);
}

int _rtp_pqueue_solve (const void *key1, const void *key2) {

  uint32_t current = ((rtp_packet_t *) key1) -> ts;
  uint32_t next = ((rtp_packet_t *) key2) -> ts;

  if (current < (uint32_t) -RTP_SOLVE_TIME)
    return (next > current);
  else
    return (next + (uint32_t) RTP_SOLVE_TIME > current + (uint32_t) RTP_SOLVE_TIME);
}

/* New member */
rtp_source_t *rtp_new_member (uint32_t ssrc) {

  rtp_source_t *member;

  member = (rtp_source_t *) alloc (sizeof(rtp_source_t));
  member -> ssrc = ssrc;

  pthread_mutex_init (&member -> guard, NULL);

  member -> max_seq = 0;
  member -> cycles = 0;
  member -> received = 0;
  member -> lost = 0;
  member -> frac = 0;
  member -> last_seq = 0;
  member -> transit = 0;
  member -> jitter = 0;
  member -> lsr = 0;
  member -> dlsr = 0;
  member -> min_seq = 0;
  member -> fail_count = 0;
  member -> cname[0] = '\0';
  member -> name[0] = '\0';
  member -> email[0] = '\0';
  member -> phone[0] = '\0';
  member -> loc[0] = '\0';
  member -> tool[0] = '\0';
  member -> note[0] = '\0';

  phtbl_init (&member -> pqueue, RTP_MAX_QUEUE, _rtp_pqueue_hash,
        _rtp_pqueue_match, free, _rtp_pqueue_solve);

  return (member);
}

/* Free member */
void rtp_free_member (rtp_source_t *ptr) {

  free (ptr);

  return;
}

/* Find member in table */
rtp_source_t *rtp_find_member (ohtbl_t *htbl, uint32_t ssrc) {

  rtp_source_t get;
  get.ssrc = ssrc;

  return ((rtp_source_t *) ohtbl_lookup(htbl, &get));
}

/* Add member to table */
int rtp_add_member (ohtbl_t *htbl, rtp_source_t *member) {

  return (ohtbl_insert (htbl, member));
}

/* Remove member from table */
rtp_source_t *rtp_remove_member (ohtbl_t *htbl, uint32_t ssrc) {

  rtp_source_t remove;
  remove.ssrc = ssrc;

  return ((rtp_source_t *) ohtbl_remove (htbl, &remove));
}

/* Parses received network data and checks for validity */
rtp_packet_t *rtp_recv (rtp_t *s, vstr_t *p) {

  uint8_t padding;
  rtp_packet_t *rtp_packet = (rtp_packet_t *) alloc (RTP_PACKET_SIZE);
  vstr_t map;
  rtp_source_t *member;

  /* maps RTP packet into vstr */
  vstr_map (&map, rtp_packet, RTP_PACKET_SIZE);
  vstr_set (&map, RTP_PACKET_IDX, RTP_PACKET_IDX);
  vstr_put_tail (&map, p -> head, p -> size);

  rtp_packet -> seq = ntohs(rtp_packet -> seq);
  rtp_packet -> ts = ntohl(rtp_packet -> ts);
  rtp_packet -> ssrc = ntohl(rtp_packet -> ssrc);

  /* check SSRC */
  pthread_mutex_lock (&s -> guard);
  member = rtp_find_member(&s -> members, rtp_packet -> ssrc);
  pthread_mutex_unlock (&s -> guard);
  if (member == NULL) {
    free (rtp_packet);
    return (NULL);
  }

  vstr_adv_head (&map, RTP_HEADER_SIZE);

  /* checks for RTP packet version */
  if(rtp_packet -> ver != RTP_VERSION) {
    free (rtp_packet);
    return (NULL);
  }

  /* check for validity of payload type:
     it must not be equal to any RTCP type */
  if (rtp_packet -> pt >= RTCP_TYPE_MIN && rtp_packet -> pt <= RTCP_TYPE_MAX) {
    free (rtp_packet);
    return (NULL);
  }

  /* checks for CSRC list and maps it */
  if (rtp_packet -> cc > 0)
    vstr_adv_head (&map, rtp_packet -> cc * sizeof(uint32_t));

  /* checks for header extension */
  if (rtp_packet -> x == 1) {
    vstr_get_head (&map, &rtp_packet -> xdef, sizeof(uint32_t));
    rtp_packet -> xdef = htons (rtp_packet -> xdef);
    rtp_packet -> xlen = htons (rtp_packet -> xlen);
    if (rtp_packet -> xlen != 0) {
      rtp_packet -> xhdr = (uint32_t *) map.head;
      vstr_adv_head (&map, rtp_packet -> xlen * sizeof(uint32_t));
    }
  }

  /* maps payload */
  rtp_packet -> payload = map.head;
  rtp_packet -> pay_len = map.size;

  /* checks for header padding */
  if (rtp_packet -> p == 1) {
    vstr_get_tail (&map, &padding, sizeof(uint8_t));

    if (padding > rtp_packet -> pay_len) {
      free (rtp_packet);
      return (NULL);
    }

    if (padding == 0) {
      free (rtp_packet);
      return (NULL);
    }

    /* updates payload lenght */
    rtp_packet -> pay_len -= padding;
  }

  pthread_mutex_lock (&member -> guard);
  rtp_update_jitter (member, rtp_packet -> ts);
  pthread_mutex_unlock (&member -> guard);

  return (rtp_packet);
}

int _rtp_enqueue_min_solve (uint16_t cur, uint16_t next) {

  if (cur > (uint16_t) RTP_MAX_QUEUE)
    return (next < cur);
  else
    return (next + (uint16_t) RTP_MAX_QUEUE < cur + (uint16_t) RTP_MAX_QUEUE);
}

int _rtp_enqueue_max_solve (uint16_t cur, uint16_t next) {

  if (cur < (uint16_t) -RTP_MAX_QUEUE)
    return (next > cur);
  else
    return (next + (uint16_t) RTP_MAX_QUEUE > cur + (uint16_t) RTP_MAX_QUEUE);
}

/* Adds RTP packet to member packet queue */
int rtp_enqueue (rtp_t *s, rtp_packet_t *rtp_packet) {

  rtp_source_t *member;

  /* check SSRC */
  pthread_mutex_lock (&s -> guard);
  member = rtp_find_member(&s -> members, rtp_packet -> ssrc);
  pthread_mutex_unlock (&s -> guard);
  if (member == NULL)
    return (-1);

  /* check for initialization */
  pthread_mutex_lock (&member -> guard);
  if (member -> fail_count >= RTP_MAX_FAIL_COUNT) {
    member -> fail_count = 0;
    member -> min_seq = rtp_packet -> seq;
    member -> max_seq = rtp_packet -> seq;
  }
  else {

    /* update minimal sequence number received */
    if (member -> min_seq == 0 || _rtp_enqueue_min_solve (member -> min_seq, rtp_packet -> seq))
      member -> min_seq = rtp_packet -> seq;

    /* update maximal sequence number received */
    if (_rtp_enqueue_max_solve (member -> max_seq, rtp_packet -> seq))
      member -> max_seq = rtp_packet -> seq;
  }

  /* update packets received from this source */
  member -> received++;

  /* update extended last sequence number received */
  member -> last_seq = rtp_packet -> seq + (member -> cycles * 65536);

  pthread_mutex_unlock (&member -> guard);

  /* insert RTP packet in queue and return */
  return (phtbl_insert (&member -> pqueue, rtp_packet));
}

/* Get (remove) packet from queue */
rtp_packet_t *rtp_get_package (phtbl_t *htbl, uint16_t seq) {

  rtp_packet_t rtp_packet;

  rtp_packet.seq = seq;
  return ((rtp_packet_t *) phtbl_remove (htbl, &rtp_packet));
}

/* Get next RTP packet from packet queue */
rtp_packet_t *rtp_dequeue (rtp_t *s, uint32_t ssrc) {

  rtp_packet_t *packet;
  rtp_source_t *member;

  /* check SSRC */
  pthread_mutex_lock (&s -> guard);
  member = rtp_find_member(&s -> members, ssrc);
  pthread_mutex_unlock (&s -> guard);
  if (member == NULL)
    return (NULL);

  pthread_mutex_lock (&member -> guard);
  if ((packet = rtp_get_package (&member -> pqueue, member -> min_seq)) == NULL) {
    member -> fail_count++;
    member -> lost++;
    if (member -> received != 0)
      member -> frac = (uint8_t) (((uint32_t) 256 * member -> lost) / member -> received);
  }
  else
    member -> fail_count = 0;

  member -> min_seq++;
  if (member -> min_seq == 0)
    member -> cycles++;
  pthread_mutex_unlock (&member -> guard);

  return (packet);
}

/* Appends a RTCP Sender Report + Receiver Reports (one for each member) */
void rtcp_send_ctrl (rtp_t *s, vstr_t *vstr) {

  struct timeval ts;
  rtcp_packet_t *rtcp_packet;
  rtp_source_t *member;

  uint32_t ntp_sec;             /* NTP time (seconds)  */
  uint32_t ntp_frac;            /* NTP time (fraction) */
  int i;                        /* just a counter      */

  /* maps RTCP packet into vstr */
  rtcp_packet = (rtcp_packet_t *) vstr -> tail;

  /* RTCP header */
  rtcp_packet -> ver = RTP_VERSION;                       /* RTP version     */
  rtcp_packet -> p = 0;                                   /* padding flag    */
  vstr_adv_tail (vstr, RTCP_HEADER_SIZE);

  pthread_mutex_lock (&s -> guard);
  rtcp_packet -> count = ohtbl_size (&s -> members);      /* report count    */
  rtcp_packet -> r.sr.ssrc = htonl(s -> ssrc);            /* SSRC of sender  */

  /* sender info */
  if (s -> flags & RTP_SENDER) {
    rtcp_packet -> pt = RTCP_SR;           /* RTCP packet is a Sender Report */
    /* calculates NTP time */
    gettimeofday(&ts, NULL);
    ntp_sec = ts.tv_sec + SECS_BETWEEN_1900_1970;
    ntp_frac = (ts.tv_usec << 12) + (ts.tv_usec << 8) - ((ts.tv_usec * 3650) >> 6);

    rtcp_packet -> r.sr.ntp_sec = htonl(ntp_sec);         /* NTP timestamp   */
    rtcp_packet -> r.sr.ntp_frac = htonl(ntp_frac);       /* NTP timestamp   */
    rtcp_packet -> r.sr.rtp_ts = htonl(ts.tv_usec + ts.tv_sec * 1000000);
    rtcp_packet -> r.sr.pcount = htonl(s -> pcount);      /* packet count    */
    rtcp_packet -> r.sr.ocount = htonl(s -> ocount);      /* octet count     */
    vstr_adv_tail (vstr, RTCP_SR_SIZE);
  }
  else
    rtcp_packet -> pt = RTCP_RR;         /* RTCP packet is a Receiver Report */

  /* report blocks */
  ohtbl_flush (&s -> members);
  for (i = 0; i < ohtbl_size (&s -> members); i++) {
    member = (rtp_source_t *) ohtbl_traverse (&s -> members);
    pthread_mutex_lock (&member -> guard);
    rtcp_packet -> r.sr.rr[i].ssrc = htonl(member -> ssrc); /* SSRC of source  */
    rtcp_packet -> r.sr.rr[i].frac = member -> frac;        /* fraction lost   */
    rtcp_packet -> r.sr.rr[i].lost = htonl(member -> lost); /* packets lost    */
    rtcp_packet -> r.sr.rr[i].last_seq = htonl(member -> last_seq);  /* ext. seq. */
    rtcp_packet -> r.sr.rr[i].jitter = htonl(member -> jitter);  /* jitter        */
    rtcp_packet -> r.sr.rr[i].lsr = htonl(member -> lsr);   /* last SR         */
    rtcp_packet -> r.sr.rr[i].dlsr = htonl(member -> dlsr); /* delay since LSR */
    pthread_mutex_unlock (&member -> guard);
    vstr_adv_tail (vstr, RTCP_RR_SIZE);
  }

  pthread_mutex_unlock (&s -> guard);

  rtcp_packet -> len = vstr -> size / 4 - 1;               /* packet length   */

  return;
}

/* Appends a RTCP Source Description report */
void rtcp_append_sdes (rtp_t *s, vstr_t *vstr, int flags) {

  rtcp_packet_t *rtcp_packet = (rtcp_packet_t *) vstr -> tail;
  rtcp_sdes_item_t *sdes_item;

  int i;                                /* just a counter */

  /* RTCP header */
  rtcp_packet -> ver = RTP_VERSION;                       /* RTP version     */
  rtcp_packet -> p = 0;                                   /* padding flag    */
  rtcp_packet -> count = 1;                               /* SDES count      */
  rtcp_packet -> pt = RTCP_SDES;                          /* PT = SDES       */
  vstr_adv_tail (vstr, RTCP_SDES_HEADER_SIZE + sizeof(uint32_t));

  pthread_mutex_lock (&s -> guard);
  rtcp_packet -> r.sdes.ssrc = htonl (s -> ssrc);         /* SSRC of sender  */

  /* SDES items */
  for (i = 0; i < RTCP_SDES_LAST; i++) {
    if (flags & (1 << i)) {

      sdes_item = (rtcp_sdes_item_t *) vstr -> tail;
      vstr_adv_tail (vstr, RTCP_SDES_ITEM_SIZE);
      sdes_item -> type = i + 1;

      switch (sdes_item -> type) {
        case RTCP_SDES_CNAME:
          sdes_item -> length = strlen (s -> cname);
          vstr_put_tail (vstr, s -> cname, strlen (s -> cname));
          break;
        case RTCP_SDES_NAME:
          sdes_item -> length = strlen (s -> name);
          vstr_put_tail (vstr, s -> name, strlen (s -> name));
          break;
        case RTCP_SDES_EMAIL:
          sdes_item -> length = strlen (s -> email);
          vstr_put_tail (vstr, s -> email, strlen (s -> email));
          break;
        case RTCP_SDES_PHONE:
          sdes_item -> length = strlen (s -> phone);
          vstr_put_tail (vstr, s -> phone, strlen (s -> phone));
          break;
        case RTCP_SDES_LOC:
          sdes_item -> length = strlen (s -> loc);
          vstr_put_tail (vstr, s -> loc, strlen (s -> loc));
          break;
        case RTCP_SDES_TOOL:
          sdes_item -> length = strlen (s -> tool);
          vstr_put_tail (vstr, s -> tool, strlen (s -> tool));
          break;
        case RTCP_SDES_NOTE:
          sdes_item -> length = strlen (s -> note);
          vstr_put_tail (vstr, s -> note, strlen (s -> note));
          break;
      }
    }
  }

  pthread_mutex_unlock (&s -> guard);

  /* end chunk */
  vstr_zero_tail (vstr, sizeof(uint32_t) - vstr -> size % sizeof(uint32_t));

  rtcp_packet -> len = vstr -> size / 4 - 1;               /* packet length   */

  return;
}

/* Parses received RTCP packet, checks for validity and updates RTP session */
int rtcp_recv (rtp_t *s, vstr_t *packet) {

  rtcp_packet_t *rtcp_packet = (rtcp_packet_t *) packet -> head;
  rtp_source_t *member;
  uint8_t rlen;
  uint32_t *ssrc;
  int i;

  while (packet -> size > 0) {

    /* check RTCP header */
    if (rtcp_packet -> ver != RTP_VERSION)
      return (-1);

    vstr_adv_head (packet, sizeof(uint32_t));

    /* parse RTCP content */
    switch (rtcp_packet -> pt) {
      case RTCP_SR:

        ssrc = (uint32_t *) packet -> head;
        *ssrc = htonl (*ssrc);
        pthread_mutex_lock (&s -> guard);
        member = rtp_find_member (&s -> members, *ssrc);

        /* update data */
        if (member != NULL) {
          pthread_mutex_unlock (&s -> guard);
          pthread_mutex_lock (&member -> guard);
          rtp_update_sr (member, packet);
          pthread_mutex_unlock (&member -> guard);
        }
        else {
          member = rtp_new_member (*ssrc);
          rtp_update_sr (member, packet);
          rtp_add_member (&s -> members, member);
          pthread_mutex_unlock (&s -> guard);
        }
        vstr_adv_head (packet, RTCP_SR_SIZE);

      case RTCP_RR:
        for (i = 0; i < rtcp_packet -> count; i++) {

          ssrc = (uint32_t *) packet -> head;
          *ssrc = htonl (*ssrc);
          pthread_mutex_lock (&s -> guard);
          member = rtp_find_member (&s -> members, *ssrc);
          pthread_mutex_unlock (&s -> guard);

          /* update data */
          if (member != NULL) {
            pthread_mutex_lock (&member -> guard);
            rtp_update_rr (member, packet);
            pthread_mutex_unlock (&member -> guard);
          }

          vstr_adv_head (packet, RTCP_RR_SIZE);
        }
        break;

      case RTCP_SDES:
        for (i = 0; i < rtcp_packet -> count; i++) {

          ssrc = (uint32_t *) packet -> head;
          *ssrc = htonl (*ssrc);
          pthread_mutex_lock (&s -> guard);
          member = rtp_find_member (&s -> members, *ssrc);
          pthread_mutex_unlock (&s -> guard);

          vstr_adv_head (packet, sizeof(uint32_t));

          /* update data */
          if (member != NULL) {
            pthread_mutex_lock (&member -> guard);
            rtp_update_sdes (member, packet);
            pthread_mutex_unlock (&member -> guard);
          }
          else
            rtp_discard_sdes (packet);
        }
        break;

      case RTCP_BYE:
        for (i = 0; i < rtcp_packet -> count; i++) {

          ssrc = (uint32_t *) packet -> head;
          *ssrc = htonl (*ssrc);
          vstr_adv_head (packet, sizeof(uint32_t));
          pthread_mutex_lock (&s -> guard);
          member = rtp_find_member (&s -> members, *ssrc);

          /* remove members */
          if (member != NULL)
            rtp_remove_member (&s -> members, *ssrc);

          pthread_mutex_unlock (&s -> guard);
        }

        if (rtcp_packet -> len > rtcp_packet -> count * sizeof(uint32_t)) {
          vstr_get_head (packet, &rlen, sizeof(uint8_t));
          vstr_adv_head (packet, rlen);
        }
        break;

      case RTCP_APP:

      default:
        return (-1);
    }

    rtcp_packet = (rtcp_packet_t *) packet -> head;
  }

  return (0);
}
