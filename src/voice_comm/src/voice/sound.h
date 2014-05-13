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

#include "global.h"

#include <stdio.h>

#include "lib/adpcm.h"
#include <fcntl.h>
#include <linux/soundcard.h>
#include <sys/ioctl.h>

#include <unistd.h>
#include <sys/time.h>
#include <signal.h>

/* Payload timer */
#define SIG_PAYLOAD_TIMER SIGALRM

/* Soundcard configuration */
/* Sample size, in bits */
#define SOUND_BITS 16

/* 1 for mono, 2 for stereo */
#define SOUND_CHANNELS 1

/* Opens a sound device and sets it, returns file descriptor */
int open_soundcard (int oflag);

/* Sound input module */
void *mod_sin(void *ptr);

/* Sound output module */
void *mod_sout(void *ptr);
