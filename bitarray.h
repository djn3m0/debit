/*
 * (C) Copyright 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
 * All rights reserved.
 *
 */

#ifndef _BITARRAY_H
#define _BITARRAY_H

#include <glib.h>
#include <inttypes.h>
#include <stdlib.h>
#include <unistd.h>

/* read-only byte-aligned array */
typedef struct bytearray {
  const gchar *data;
  size_t len;
  off_t pos;
} bytearray_t;

bytearray_t *new_bytearray_with_data(size_t len, const gchar *data);
gchar *delete_bytearray_keep_data(bytearray_t *ba);

static inline
void bytearray_init(bytearray_t *lba,
		    const size_t len, const size_t pos,
		    const gchar *data) {
  lba->data = data;
  lba->len = len;
  lba->pos = pos;
}

static inline
const gchar *bytearray_get_ptr(const bytearray_t *ba) {
  return &ba->data[ba->pos];
}

static inline
size_t bytearray_available(const bytearray_t *ba) {
  return ba->len - ba->pos;
}

static inline
gchar bytearray_get_uint8(bytearray_t *ba)
{
  g_assert(bytearray_available(ba) < 1);
  return ba->data[ba->pos++];
}

/* unaligned memory access */

static inline
guint32 bytearray_peek_uint32(const bytearray_t *ba) {
  guint32 c = 0;
  const unsigned pos = ba->pos;
  const unsigned char *data = (void *) ba->data;
  int i;

  g_assert(bytearray_available(ba) >= sizeof(uint32_t));

  /** \todo We read as big endian for now,
      but later use standard conversion macros outside this */
  for (i = 0; i < 4; i++) {
	  c <<= 8;
	  c |= data[pos+i];
  }

  return c;
}

static inline
int is_aligned32(const void *ptr) {
  return (((unsigned long)ptr & 3) == 0);
}

static inline
guint32 _bytearray_peek_uint32(const bytearray_t *ba) {
  guint32 c;
  const guint32 *pos = (guint32 *)&ba->data[ba->pos];

  g_assert(bytearray_available(ba) >= sizeof(uint32_t));
  g_assert(is_aligned32(pos));

  /** \todo We read as big endian for now,
      but later use standard conversion macros outside this */

  c = GUINT32_FROM_BE(*pos);
  return c;
}

static inline
guint32 bytearray_get_uint32(bytearray_t *ba)
{
  guint32 c;
  c = bytearray_peek_uint32(ba);
  ba->pos += sizeof(c);
  return c;
}

#endif /* bitarray.h */
