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

#include "global.h"
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>

/* Port of socket */
#define RTP_PORT        5001
#define RTCP_PORT       5002

/* Communication input module */
void *mod_cin(void *ptr);

/* Communication output module */
void *mod_cout(void *ptr);

