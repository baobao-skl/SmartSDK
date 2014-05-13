/* global.h - Global variables and definitions
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

#include "lib/alloc.h"
#include "lib/vstr.h"
#include "lib/rtp.h"

/* Defines */
/* Samples to read from soundcard (16-bit, 8000 Hz) */
#define SAMPLES 320

#ifndef UDP_DATA
#define UDP_DATA
/* Data struct */
struct udp_data
{
  int seq;			/* Sequence number */
  char block[SAMPLES/2];	/* Data block */
};
#endif

/* Global */
rtp_t rtp;                      /* RTP session */
