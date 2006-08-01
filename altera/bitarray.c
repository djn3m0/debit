/*
   Copyright (C) 2003 rzyjontko
   Copyright (C) 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>

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

#include <glib.h>
#include "bitarray.h"

#define WORD_BITSIZE (8 * sizeof(array_storage_t))

static void
print_int (int a)
{
  g_print (" %d", a);
}

bitarray_t *
bitarray_create_data (char *data, int bits) {
  bitarray_t *result;

  result        = g_new0 (bitarray_t, 1);
  result->bits  = bits;
  result->size  = (bits + WORD_BITSIZE - 1) / WORD_BITSIZE;
  result->array = (void*) data;
  return result;
}

bitarray_t *
bitarray_create (int bits)
{
  bitarray_t *result;

  if (bits == 0)
    return NULL;

  result = bitarray_create_data(NULL, bits);
  result->array = g_new0 (array_storage_t, result->size);
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

array_storage_t *
bitarray_free (bitarray_t *a, gboolean keep_data) {
  array_storage_t *data = a->array;
  g_assert(a);

  g_free(a);

  if (keep_data)
    return data;

  g_free(data);
  return NULL;
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
  if (bit > a->bits)
    return;

  a->array[bit / WORD_BITSIZE] |= (1 << (bit % WORD_BITSIZE));
}



void
bitarray_unset (bitarray_t *a, int bit)
{
  if (bit > a->bits)
    return;

  a->array[bit / WORD_BITSIZE] &= ~ (1 << (bit % WORD_BITSIZE));
}



void
bitarray_change_bit (bitarray_t *a, int bit)
{
  if (a == NULL)
    return;

  if (bit > a->bits)
    return;

  a->array[bit / WORD_BITSIZE] ^= (1 << (bit % WORD_BITSIZE));
}



void
bitarray_neg (bitarray_t *a)
{
  int i;

  for (i = 0; i < a->size; i++)
    a->array[i] ^= ~ 0;
}



void
bitarray_sum (bitarray_t *a, const bitarray_t *b)
{
  int i;

  for (i = 0; i < a->size; i++)
    a->array[i] |= b->array[i];
}



void
bitarray_intersect (bitarray_t *a, const bitarray_t *b)
{
  int i;

  for (i = 0; i < a->size; i++)
    a->array[i] &= b->array[i];
}

void
bitarray_subtract (bitarray_t *a, const bitarray_t *b)
{
  int i;

  for (i = 0; i < a->size; i++)
    a->array[i] &= ~ b->array[i];
}

void
bitarray_diffsym (bitarray_t *a, const bitarray_t *b)
{
  int i;

  for (i = 0; i < a->size; i++)
    a->array[i] ^= b->array[i];
}

int
bitarray_equal (const bitarray_t *a, const bitarray_t *b)
{
  int i;

  for (i = 0; i < a->size; i++)
    if (a->array[i] != b->array[i])
      return 0;
  return 1;
}



void
bitarray_copy (bitarray_t *to, const bitarray_t *from)
{
  int i;

  for (i = 0; i < to->size; i++)
    to->array[i] = from->array[i];
}



int
bitarray_is_set (const bitarray_t *a, int bit)
{
  if (a && a->array){
    return a->array[bit / 8 * sizeof(array_storage_t)] & (1 << (bit % 8 * sizeof(array_storage_t)));
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
    for (j = 0; j < WORD_BITSIZE && WORD_BITSIZE * i + j < a->bits; j++){
      if ((a->array[i] & (1 << j)) != 0)
        fun (WORD_BITSIZE * i + j);
    }
  }
}



void
bitarray_print (bitarray_t *a)
{
  bitarray_for_ones (a, print_int);
  g_print ("\n");
}



int
bitarray_ones_count (const bitarray_t *a)
{
  unsigned char x;
  int      i;
  int      result = 0;

  for (i = 0; i < a->size; i++){
    x = a->array[i];
    while (x) {
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
    for (j = 0; j < WORD_BITSIZE && WORD_BITSIZE * i + j < a->bits; j++){
      if ((a->array[i] & (1 << j)) != 0)
        if (! fun (WORD_BITSIZE * i + j))
          return 0;
    }
  }
  return 1;
}



int
bitarray_first_set (const bitarray_t *a)
{
  int i;
  int j;

  for (i = 0; i < a->size; i++){
    for (j = 0; j < WORD_BITSIZE && WORD_BITSIZE * i + j < a->bits; j++){
      if ((a->array[i] & (1 << j)) != 0)
        return WORD_BITSIZE * i + j;
    }
  }
  return -1;
}



int
bitarray_none_is_set (const bitarray_t *a)
{
  int i;

  for (i = 0; i < a->size; i++){
    if (a->array[i])
      break;
  }
  return i == a->size;
}

