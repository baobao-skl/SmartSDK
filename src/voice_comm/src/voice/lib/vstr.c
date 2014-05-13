/* vstr.c - Virtual Stream                                                */
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

#include "vstr.h"
#include <string.h>

void vstr_init (vstr_t *vstr, int max) {

  vstr -> max = max;
  vstr -> size = 0;
  vstr -> data = (char *) alloc (max);
  vstr -> head = vstr -> data;
  vstr -> tail = vstr -> data;

  return;
}

void vstr_map (vstr_t *vstr, void *data, int size) {

  vstr -> max = size;
  vstr -> size = size;
  vstr -> data = (char *) data;
  vstr -> head = vstr -> data;
  vstr -> tail = vstr -> data + size;

  return;
}

void vstr_free (vstr_t *vstr) {

  free (vstr -> data);

  return;
}

void vstr_flush (vstr_t *vstr) {

  vstr -> size = 0;
  vstr -> head = vstr -> data;
  vstr -> tail = vstr -> data;

  return;
}

void vstr_set (vstr_t *vstr, int head, int tail) {

  vstr -> head = vstr -> data + head;
  vstr -> tail = vstr -> data + tail;
  vstr -> size = tail - head;

  return;
}

void vstr_adv_head (vstr_t *vstr, int len) {

  vstr -> head += len;
  vstr -> size -= len;

  return;
}

void vstr_adv_tail (vstr_t *vstr, int len) {

  vstr -> tail += len;
  vstr -> size += len;

  return;
}

void vstr_rev_tail (vstr_t *vstr, int len) {

  vstr -> tail -= len;
  vstr -> size -= len;

  return;
}

void vstr_rev_head (vstr_t *vstr, int len) {

  vstr -> head -= len;
  vstr -> size += len;

  return;
}

void vstr_put_tail (vstr_t *vstr, void *data, int size) {

  memcpy (vstr -> tail, data, size);
  vstr -> tail += size;
  vstr -> size += size;

  return;
}

void vstr_put_head (vstr_t *vstr, void *data, int size) {

  vstr -> head -= size;
  memcpy (vstr -> head, data, size);
  vstr -> size += size;

  return;
}

void vstr_get_head (vstr_t *vstr, void *data, int size) {

  memcpy (data, vstr -> head, size);
  vstr -> head += size;
  vstr -> size -= size;

  return;
}

void vstr_get_tail (vstr_t *vstr, void *data, int size) {

  vstr -> tail -= size;
  memcpy (data, vstr -> tail, size);
  vstr -> size -= size;

  return;
}

void vstr_zero_tail (vstr_t *vstr, int len) {

  memset (vstr -> tail, '\0',  len);
  vstr_adv_tail (vstr, len);

  return;
}

void vstr_zero_head (vstr_t *vstr, int len) {

  vstr_rev_head (vstr, len);
  memset (vstr -> head, '\0', len);

  return;
}
