/*
   elmo - ELectronic Mail Operator

   Copyright (C) 2003 rzyjontko

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

   ----------------------------------------------------------------------

   This file contains an implementation of bitarrays - arrays of bits,
   efficient representation of sets.

*/
/****************************************************************************
 *    IMPLEMENTATION HEADERS
 ****************************************************************************/

#include <stdio.h>
#include <glib.h>
#include "bitarray.h"

static void
print_int (int a)
{
  printf (" %d", a);
}

/****************************************************************************
 *    INTERFACE FUNCTIONS
 ****************************************************************************/

bitarray_t *
bitarray_create_data (char *data, int bits) {
  bitarray_t *result;

  result        = g_new0 (bitarray_t, 1);
  result->bits  = bits;
  result->size  = (bits + sizeof(unsigned) * 8 - 1) / (sizeof(unsigned)*8);
  result->array  = (unsigned *)data;
  return result;
}

bitarray_t *
bitarray_create (int bits)
{
  bitarray_t *result;

  if (bits == 0)
    return NULL;

  result = bitarray_create_data(NULL, bits);
  result->array = g_new (unsigned, result->size);
  return result;
}

bitarray_t *
bitarray_reverse (bitarray_t *b)
{
  int         i;
  bitarray_t *result;

  result = bitarray_create (b->bits);
  bitarray_ones (result);
  for (i = 0; i < result->size; i++)
    result->array[i] ^= b->array[i];
  return result;
}

void
bitarray_remove (bitarray_t *a, int index)
{
  if (bitarray_is_set (a, a->bits - 1))
    bitarray_set (a, index);
  else
    bitarray_unset (a, index);

  a->bits--;
}



void
bitarray_destroy (bitarray_t *a)
{
  if (a == NULL)
    return;

  if (a && a->array)
    g_free (a->array);
  if (a)
    g_free (a);
}



void
bitarray_zeros (bitarray_t *a)
{
  int i;

  if (a == NULL)
    return;

  for (i = 0; i < a->size; i++)
    a->array[i] ^= a->array[i];
}



void
bitarray_ones (bitarray_t *a)
{
  int i;

  if (a == NULL)
    return;

  for (i = 0; i < a->size; i++)
    a->array[i] |= ~a->array[i];
}



void
bitarray_set (bitarray_t *a, int bit)
{
  if (a == NULL)
    return;

  if (bit > a->bits)
    return;

  a->array[bit / 32] |= (1 << (bit % 32));
}



void
bitarray_unset (bitarray_t *a, int bit)
{
  if (a == NULL)
    return;

  if (bit > a->bits)
    return;

  a->array[bit / 32] &= ~ (1 << (bit % 32));
}



void
bitarray_change_bit (bitarray_t *a, int bit)
{
  if (a == NULL)
    return;

  if (bit > a->bits)
    return;

  a->array[bit / 32] ^= (1 << (bit % 32));
}



void
bitarray_neg (bitarray_t *a)
{
  int i;

  for (i = 0; i < a->size; i++)
    a->array[i] ^= ~ 0;
}



void
bitarray_sum (bitarray_t *a, bitarray_t *b)
{
  int i;

  for (i = 0; i < a->size; i++)
    a->array[i] |= b->array[i];
}



void
bitarray_intersect (bitarray_t *a, bitarray_t *b)
{
  int i;

  for (i = 0; i < a->size; i++)
    a->array[i] &= b->array[i];
}



void
bitarray_subtract (bitarray_t *a, bitarray_t *b)
{
  int i;

  for (i = 0; i < a->size; i++)
    a->array[i] &= ~ b->array[i];
}


int
bitarray_equal (bitarray_t *a, bitarray_t *b)
{
  int i;

  for (i = 0; i < a->size; i++)
    if (a->array[i] != b->array[i])
      return 0;
  return 1;
}



void
bitarray_copy (bitarray_t *to, bitarray_t *from)
{
  int i;

  for (i = 0; i < to->size; i++)
    to->array[i] = from->array[i];
}



int
bitarray_is_set (bitarray_t *a, int bit)
{
  if (a && a->array){
    return a->array[bit / 32] & (1 << (bit % 32));
  }
  else
    return 0;
}



void
bitarray_for_ones (bitarray_t *a, void (*fun)(int))
{
  int i, j;

  if (a == NULL)
    return;

  for (i = 0; i < a->size; i++){
    for (j = 0; j < 32 && 32 * i + j < a->bits; j++){
      if ((a->array[i] & (1 << j)) != 0)
        fun (32 * i + j);
    }
  }
}



void
bitarray_print (bitarray_t *a)
{
  bitarray_for_ones (a, print_int);
  printf ("\n");
}



int
bitarray_ones_count (bitarray_t *a)
{
  unsigned x;
  int      i;
  int      result = 0;

  for (i = 0; i < a->size; i++){
    x = a->array[i];
    while (x){
      x &= x - 1;
      result++;
    }
  }
  return result;
}



int
bitarray_true_for_all (bitarray_t *a, int (*fun)(int))
{
  int i;
  int j;

  for (i = 0; i < a->size; i++){
    for (j = 0; j < 32 && 32 * i + j < a->bits; j++){
      if ((a->array[i] & (1 << j)) != 0)
        if (! fun (32 * i + j))
          return 0;
    }
  }
  return 1;
}



int
bitarray_first_set (bitarray_t *a)
{
  int i;
  int j;

  for (i = 0; i < a->size; i++){
    for (j = 0; j < 32 && 32 * i + j < a->bits; j++){
      if ((a->array[i] & (1 << j)) != 0)
        return 32 * i + j;
    }
  }
  return -1;
}



int
bitarray_none_is_set (bitarray_t *a)
{
  int i;

  for (i = 0; i < a->size; i++){
    if (a->array[i])
      break;
  }
  return i == a->size;
}

/****************************************************************************
 *    INTERFACE CLASS BODIES
 ****************************************************************************/
/****************************************************************************
 *
 *    END MODULE bitarray.c
 *
 ****************************************************************************/
