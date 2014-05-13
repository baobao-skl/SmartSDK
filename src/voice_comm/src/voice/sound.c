/* sound.h - Sound input/output modules
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

#include "sound.h"
#include <stdio.h>

extern rtp_t rtp;

void *mod_sout (void *ptr) {

  short outbuf[SAMPLES];	/* Output buffer to get sound samples */

  int soundfd;			/* Sound device file descriptor */
  struct adpcm_state state;	/* ADPCM state, see adpcm.c */

  struct itimerval interval;	/* Interval to set frequency */

  sigset_t allsigs;		/* Signal set */
  int fail = 10;

  rtp_packet_t *packet;
  rtp_source_t *member;

  fprintf(stderr, "+ Sound output module loaded.\n");

  /* Initialize ADPCM encoder */
  state.valprev = 0;
  state.index = 0; 

  /* Open sound device to write */
  soundfd = open_soundcard (O_WRONLY | O_SYNC);

  /* Block signals */
  sigemptyset (&allsigs);
  sigaddset (&allsigs, SIG_PAYLOAD_TIMER);
  sigprocmask (SIG_BLOCK, &allsigs, NULL);

  /* Set timer */
  interval.it_value.tv_sec = 0;
  interval.it_value.tv_usec = 40000;
  interval.it_interval.tv_sec = 0;
  interval.it_interval.tv_usec = 40000;
  setitimer (ITIMER_REAL, &interval, NULL);

  for (;;) 
  {
    sigwaitinfo (&allsigs, NULL);

    /* Get data from buffer */
    ohtbl_flush (&rtp.members);
    if ((member = (rtp_source_t *) ohtbl_traverse (&rtp.members)) != NULL) {

      packet = rtp_dequeue (&rtp, member -> ssrc);

      /* Decodes data */
      if (packet != NULL) {
        fail = 0;
        adpcm_decoder (packet -> payload, outbuf, SAMPLES, &state);
        rtp_packet_free (packet);
      }
        fail++;

      if (fail < 10) {
        /* Send it to sound device */
        write (soundfd, outbuf, sizeof(outbuf));
      }
    }
  }
}

int open_soundcard (int oflag) {

  int fd;
  int arg;
  int status;

  fd = open("/dev/dsp", oflag);
  if (fd < 0) {
    perror ("open of /dev/dsp failed");
    exit (1);
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
