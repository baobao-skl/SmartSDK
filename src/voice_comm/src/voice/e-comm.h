/* e-comm.h - Includes and prototypes
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
#ifndef __ECOMM_H__
#define __ECOMM_H__

#include "global.h"

#include <stdio.h>
#include <stdlib.h>

#include <ctype.h>

#include <pthread.h>
#include <signal.h>

#include "sound.h"
#include "comm.h"

#define DEBUG
#ifdef DEBUG
	#define DEBUG_MSG printf
#else
	#define DEBUG_MSG(...)
#endif

#endif

