/* ohtbl.c - open-addressed hash tables                                   */
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

/* code is based on "Mastering Algorithms with C", Kyle Loudon */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ohtbl.h"

/* Reserve a sentinel memory address for vacated elements */
static char vacated;

/* ohtbl_init */
int ohtbl_init(ohtbl_t *htbl, unsigned int positions, unsigned int (*h1)(const void *key),
   unsigned int (*h2)(const void *key), int (*match)(const void *key1, const void *key2),
   void (*destroy)(void *data), unsigned int tolerance) {

  int i;

  htbl -> table = (struct ohtbl_item *) malloc ((positions + 1) * sizeof(struct ohtbl_item));
  if (htbl -> table == NULL)
    return (-1);

  /* Initialize tolerance */
  htbl -> tolerance = tolerance;

  /* Initialize each position */
  htbl -> positions = positions;

  for (i = 0; i < htbl -> positions; i++)
    htbl -> table[i].data = NULL;

  htbl -> head = &htbl -> table[positions];
  htbl -> tail = &htbl -> table[positions];

  /* Set the vacated member to the sentinel memory address reserved for this */
  htbl -> vacated = &vacated;

  /* Encapsulate the functions */
  htbl -> h1 = h1;
  htbl -> h2 = h2;
  htbl -> match = match;
  htbl -> destroy = destroy;

  /* Initialize the number of elements in the table */
  htbl -> size = 0;

  return (0);
}

/* ohtbl_destroy */
void ohtbl_destroy(ohtbl_t *htbl) {

  int i;

  if (htbl -> destroy != NULL) {

    /* Call a user-defined function to free dynamically allocated data */
    for (i = 0; i < htbl -> positions; i++) {
       if (htbl -> table[i].data != NULL && htbl -> table[i].data != htbl -> vacated)
          htbl -> destroy (htbl -> table[i].data);
    }
  }

  /* Free the storage allocated for the hash table */
  free (htbl -> table);

  /* No operations are allowed now, but clear the structure as a precaution */
  memset (htbl, 0, sizeof(ohtbl_t));

  return;
}

/* ohtbl_insert */
int ohtbl_insert(ohtbl_t *htbl, void *data) {

  unsigned int position, i;

  /* Do not exceed the tolerance in the table */
  if (htbl -> size == htbl -> tolerance)
    return (-1);

  /* Do nothing if the data is already in the table */
  if (ohtbl_lookup (htbl, data) != NULL)
    return (1);

  /* Use double hashing to hash the key */
  for (i = 0; i < htbl -> positions; i++) {
    position = (htbl -> h1 (data) + (i * htbl -> h2 (data))) % htbl -> positions;
    if (htbl -> table[position].data == NULL || htbl -> table[position].data == htbl-> vacated) {

      /* Insert the data into the table */
      htbl -> table[position].data = data;
      htbl -> size++;

      /* Append element to linked list */
      htbl -> tail -> next = &htbl -> table[position];
      htbl -> table[position].prev = htbl -> tail;
      htbl -> tail = htbl -> tail -> next;

      return (0);
    }
  }

  /* Return that the hash functions were selected incorrectly */
  return (-2);
}

/* Flush current position for traversing */
void ohtbl_flush (ohtbl_t *htbl) {

  htbl -> cur = htbl -> head;

  return;
}

/* Traverse hash table */
void *ohtbl_traverse (ohtbl_t *htbl) {

  if (htbl -> cur == htbl -> tail)
    return (NULL);

  htbl -> cur = htbl -> cur -> next;

  return (htbl -> cur -> data);
}

/* ohtbl_remove */
void *ohtbl_remove(ohtbl_t *htbl, const void *data) {

  unsigned int position, i;

  /* Use double hashing to hash the key */
  for (i = 0; i < htbl -> positions; i++) {
    position = (htbl -> h1 (data) + (i * htbl -> h2 (data))) % htbl -> positions;

    /* Return that the data was not found */
    if (htbl -> table[position].data == NULL)
      return (NULL);

    /* Search beyond vacated positions */
    else if (htbl -> table[position].data == htbl -> vacated)
      continue;

    /* Pass back the data from the table */
    else if (htbl -> match (htbl -> table[position].data, data)) {
      htbl -> size--;

      /* Update linked list */
      htbl -> table[position].prev -> next = htbl -> table[position].next;
      htbl -> table[position].next -> prev = htbl -> table[position].prev;

      /* Put sentinel */
      htbl -> table[position].data = htbl -> vacated;

      return (htbl -> table[position].data);
    }
  }

  /* Return that the data was not found */
  return (NULL);
}

/* ohtbl_lookup */
void *ohtbl_lookup (const ohtbl_t *htbl, const void *data) {

  unsigned int position, i;

  /* Use double hashing to hash the key */
  for (i = 0; i < htbl -> positions; i++) {
    position = (htbl -> h1 (data) + (i * htbl -> h2 (data))) % htbl -> positions;

    /* Return that the data was not found */
    if (htbl -> table[position].data == NULL)
      return (NULL);

    /* Data was found */
    else if (htbl -> match (htbl -> table[position].data, data))
      return (htbl -> table[position].data);
  }

  /* Return that the data was not found */
  return (NULL);
}
