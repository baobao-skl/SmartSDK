/* phtbl.h - perfect hash table (pthread safe)                            */
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

#ifndef PHTBL_H
#define PHTBL_H

#include <pthread.h>
#include <stdlib.h>

/* Define a structure pthread safe */
struct phtbl_item {
  void *data;                                        /* data pointer      */
  pthread_mutex_t guard;                             /* data guardian     */
};

/* Define a structure for open-addressed hash tables */
typedef struct {
  unsigned int positions;                            /* max positions     */
  unsigned int (*hash) (const void *key);            /* aux. hash func    */
  int (*solve) (const void *key1, const void *key2); /* solve collision   */
  int (*match) (const void *key1, const void *key2); /* match function    */
  void (*destroy) (void *data);                      /* free data         */
  struct phtbl_item *table;                          /* hash table        */
} phtbl_t;                                           /* perfect hash tbl  */

/* positions can be any positive integer number */
int phtbl_init(phtbl_t *htbl, unsigned int positions, unsigned int (*hash)(const void *key),
   int (*match)(const void *key1, const void *key2), void (*destroy)(void *data),
   int (*solve)(const void *key1, const void *key2));

void phtbl_clean (phtbl_t *htbl);

void phtbl_destroy(phtbl_t *htbl);

int phtbl_insert(phtbl_t *htbl, void *data);

void *phtbl_remove(phtbl_t *htbl, const void *data);

void *phtbl_lookup(const phtbl_t *htbl, const void *data);

#endif /* PHTBL_H */
