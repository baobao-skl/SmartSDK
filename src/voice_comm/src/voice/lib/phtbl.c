/* phtbl.c - perfect hash table (pthread safe)                            */
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

#include "phtbl.h"
#include <string.h>

/* phtbl_init */
int phtbl_init(phtbl_t *htbl, unsigned int positions, unsigned int (*hash)(const void *key),
   int (*match)(const void *key1, const void *key2), void (*destroy)(void *data),
   int (*solve)(const void *key1, const void *key2)) {

  int i;

  htbl -> table = (struct phtbl_item *) malloc (positions * sizeof(struct phtbl_item));
  if (htbl -> table == NULL)
    return (-1);

  /* Initialize each position */
  htbl -> positions = positions;

  for (i = 0; i < htbl -> positions; i++) {
    htbl -> table[i].data = NULL;
    pthread_mutex_init (&htbl -> table[i].guard, NULL);
  }

  /* Encapsulate the functions */
  htbl -> hash = hash;
  htbl -> match = match;
  htbl -> destroy = destroy;
  htbl -> solve = solve;

  return (0);
}

/* phtbl_clean */
void phtbl_clean (phtbl_t *htbl) {

  int i;

  for (i = 0; i < htbl -> positions; i++) {
    pthread_mutex_lock (&htbl -> table[i].guard);
    if (htbl -> table[i].data != NULL && htbl -> destroy != NULL)
      htbl -> destroy (htbl -> table[i].data);
    htbl -> table[i].data = NULL;
    pthread_mutex_unlock (&htbl -> table[i].guard);
  }

  return;
}

/* phtbl_destroy */
void phtbl_destroy (phtbl_t *htbl) {

  int i;

  /* Call a user-defined function to free dynamically allocated data */
  for (i = 0; i < htbl -> positions; i++) {
     if (htbl -> destroy != NULL && htbl -> table[i].data != NULL)
        htbl -> destroy (htbl -> table[i].data);
    pthread_mutex_destroy (&htbl -> table[i].guard);
  }

  /* Free the storage allocated for the hash table */
  free (htbl -> table);

  /* No operations are allowed now, but clear the structure as a precaution */
  memset (htbl, 0, sizeof(phtbl_t));

  return;
}

/* phtbl_insert */
int phtbl_insert(phtbl_t *htbl, void *data) {

  /* Use hashing function to hash the key */
  unsigned int position = htbl -> hash (data) % htbl -> positions;
  int ret = -1;

  /* Lock guardian */
  pthread_mutex_lock (&htbl -> table[position].guard);

  /* Solve collision if there is already data in this position */
  if (htbl -> table[position].data != NULL) {
    if (htbl -> solve (htbl -> table[position].data, data)) {
      if (htbl -> destroy != NULL)
        htbl -> destroy (htbl -> table[position].data);
      htbl -> table[position].data = NULL;
    }
  }

  /* Add key if there is space for it */
  if (htbl -> table[position].data == NULL) {
    htbl -> table[position].data = data;
    ret = 0;
  }

  /* Unlock guardian */
  pthread_mutex_unlock (&htbl -> table[position].guard);

  /* Return that the current data cannot be inserted in table */
  return (ret);
}

/* phtbl_remove */
void *phtbl_remove(phtbl_t *htbl, const void *data) {

  /* Use hashing function to hash the key */
  unsigned int position = htbl -> hash (data) % htbl -> positions;
  void *temp = NULL;

  /* Lock guardian */
  pthread_mutex_lock (&htbl -> table[position].guard);

  /* Check if table has element in that position */
  if (htbl -> table[position].data != NULL) {

    /* Check if data matches */
    if (htbl -> match (htbl -> table[position].data, data)) {

      /* Clean current position */
      temp = htbl -> table[position].data;
      htbl -> table[position].data = NULL;
    }
  }

  /* Unlock guardian */
  pthread_mutex_unlock (&htbl -> table[position].guard);

  /* Return that the data was not found */
  return (temp);
}

/* phtbl_lookup */
void *phtbl_lookup (const phtbl_t *htbl, const void *data) {

  /* Use hashing function to hash the key */
  unsigned int position = htbl -> hash (data) % htbl -> positions;
  void *temp = NULL;

  /* Lock guardian */
  pthread_mutex_lock (&htbl -> table[position].guard);

  /* Data was found */
  if (htbl -> match (htbl -> table[position].data, data))
    temp = htbl -> table[position].data;

  /* Unlock guardian */
  pthread_mutex_unlock (&htbl -> table[position].guard);

  /* Return that the data was not found */
  return (temp);
}
