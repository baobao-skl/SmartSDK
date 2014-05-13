/* vstr.h - Virtual Stream                                                */
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

#ifndef VSTR_H
#define VSTR_H

#include "alloc.h"

typedef struct {
  int max;                      /* max size of stream   */
  int size;                     /* size of stream       */
  char *head;                   /* head of stream       */
  char *tail;                   /* tail of stream       */
  char *data;                   /* data                 */
} vstr_t;

void vstr_init (vstr_t *vstr, int max);

void vstr_map (vstr_t *vstr, void *data, int size);

void vstr_free (vstr_t *vstr);

void vstr_flush (vstr_t *vstr);

void vstr_set (vstr_t *vstr, int head, int tail);

void vstr_adv_head (vstr_t *vstr, int len);

void vstr_adv_tail (vstr_t *vstr, int len);

void vstr_rev_tail (vstr_t *vstr, int len);

void vstr_rev_head (vstr_t *vstr, int len);

void vstr_put_head (vstr_t *vstr, void *data, int size);

void vstr_put_tail (vstr_t *vstr, void *data, int size);

void vstr_get_head (vstr_t *vstr, void *data, int size);

void vstr_get_tail (vstr_t *vstr, void *data, int size);

void vstr_zero_tail (vstr_t *vstr, int len);

void vstr_zero_head (vstr_t *vstr, int len);

#endif /* VSTR_H */
