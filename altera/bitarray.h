#ifndef __BITARRAY_H__
#define __BITARRAY_H__ 1
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

*/

typedef struct bitarray {
  unsigned *array;
  int       size;
  int       bits;
} bitarray_t;

extern bitarray_t *bitarray_create (int bits);
extern bitarray_t *bitarray_create_data (char *data, int bits);
extern bitarray_t *bitarray_reverse (bitarray_t *b);
extern void        bitarray_destroy (bitarray_t *a);
extern void        bitarray_remove (bitarray_t *a, int index);
extern void        bitarray_zeros (bitarray_t *a);
extern void        bitarray_ones (bitarray_t *a);
extern void        bitarray_set (bitarray_t *a, int bit);
extern void        bitarray_unset (bitarray_t *a, int bit);
extern void        bitarray_change_bit (bitarray_t *a, int bit);
extern void        bitarray_neg (bitarray_t *a);
extern void        bitarray_sum (bitarray_t *a, bitarray_t *b);
extern void        bitarray_intersect (bitarray_t *a, bitarray_t *b);
extern void        bitarray_subtract (bitarray_t *a, bitarray_t *b);
extern int         bitarray_equal (bitarray_t *a, bitarray_t *b);
extern void        bitarray_copy (bitarray_t *to, bitarray_t *from);
extern int         bitarray_is_set (bitarray_t *a, int bit);
extern void        bitarray_for_ones (bitarray_t *a, void (*fun)(int));
extern void        bitarray_print (bitarray_t *a);
extern int         bitarray_ones_count (bitarray_t *a);
extern int         bitarray_true_for_all (bitarray_t *a, int (*fun)(int));
extern int         bitarray_first_set (bitarray_t *a);
extern int         bitarray_none_is_set (bitarray_t *a);

/****************************************************************************
 *    INTERFACE OBJECT CLASS DEFINITIONS
 ****************************************************************************/
/****************************************************************************
 *    INTERFACE TRAILING HEADERS
 ****************************************************************************/
/****************************************************************************
 *
 *    END HEADER bitarray.h
 *
 ****************************************************************************/
#endif
