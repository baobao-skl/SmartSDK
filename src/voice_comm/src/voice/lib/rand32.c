/* rand32.c - random 32-bit number generator
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#include "rand32.h"

#include <time.h>
#include <sys/time.h>
#include "md5.h"
#include "mwc1616.h"

#define MD_CTX MD5_CTX
#define MDInit MD5Init
#define MDUpdate MD5Update
#define MDFinal MD5Final

static unsigned long md_32 (char *string, int lenght) {

  MD_CTX context;
  union {
    char c[16];
    unsigned long x[4];
  } digest;
  unsigned long r;
  int i;

  MDInit (&context);
  MDUpdate (&context, string, lenght);
  MDFinal ((unsigned char *) &digest, &context);
  r = 0;
  for (i = 0; i < 3; i++) {
    r ^= digest.x[i];
  }

  return (r);
}

unsigned long rand32 (void) {

  struct {
    clock_t cpu;
    unsigned long mwc1616_x;
    unsigned long mwc1616_y;
  } s;

  struct timeval ts;

  gettimeofday (&ts, NULL);
  seed_rand_mwc1616 (ts.tv_usec);
  s.mwc1616_x = rand_mwc1616();
  seed_rand_mwc1616 (ts.tv_sec);
  s.mwc1616_y = rand_mwc1616();
  s.cpu = clock();

  return (md_32((char *) &s, sizeof(s)));
}
