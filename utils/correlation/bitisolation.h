/*
 * (C) Copyright 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
 * All rights reserved.
 *
 */

#ifndef _HAS_BITISOLATION_H
#define _HAS_BITISOLATION_H

#include <string.h>
#include <glib.h>
#include "altera/bitarray.h"

typedef struct state {
  bitarray_t *known_data;
  bitarray_t *unknown_data;
} state_t;

typedef struct alldata {
  /** length in bytes of the known data array */
  size_t known_data_len;
  /** length in bytes of the unknown data array */
  size_t unknown_data_len;
  unsigned width;
  size_t nstates;
  state_t *states;
} alldata_t;

static inline int
alloc_state(state_t *to, const alldata_t *dat) {
  size_t len = dat->known_data_len;
  size_t ulen = dat->unknown_data_len;
  to->known_data = bitarray_create(8 * len);
  to->unknown_data = bitarray_create(8 * ulen);
  return 0;
}

static inline void
init_state(state_t *to) {
  bitarray_ones(to->known_data);
  bitarray_ones(to->unknown_data);
}

static inline void
release_state(state_t *to) {
  (void) bitarray_free(to->known_data, FALSE);
  (void) bitarray_free(to->unknown_data, FALSE);
}

static inline void
and_state(const state_t *s1, const state_t *s2) {
  bitarray_intersect(s1->known_data, s2->known_data);
  bitarray_intersect(s1->unknown_data, s2->unknown_data);
}

static inline void
or_state(const state_t *s1, const state_t *s2) {
  bitarray_sum (s1->known_data, s2->known_data);
  bitarray_sum (s1->unknown_data, s2->unknown_data);
}

static inline void
and_neg_state(state_t *s1, const state_t *s2) {
  bitarray_subtract (s1->known_data, s2->known_data);
  bitarray_subtract (s1->unknown_data, s2->unknown_data);
}

static inline int
bit_is_present(const unsigned bit, const state_t *s) {
  /* bitarray lookup in unknown data */
  return bitarray_is_set (s->unknown_data, bit);
}

static inline int
known_bit_is_present(const unsigned bit, const state_t *s) {
  return bitarray_is_set (s->known_data, bit);
}

/* find first non-nil */
static inline int
unk_data_nil(const state_t *s) {
  return bitarray_none_is_set (s->unknown_data);
}

/* */
static inline int
is_isolated(const state_t *s) {
  unsigned count;
  count = bitarray_ones_count (s->known_data);
  return (count == 1);
}

#endif /* _HAS_BITISOLATION_H */
