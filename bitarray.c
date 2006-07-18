/*
 * (C) Copyright 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
 * All rights reserved.
 *
 */


#include <glib.h>
#include "bitarray.h"

bytearray_t *new_bytearray_with_data(const size_t len, const gchar *data) {
  bytearray_t *lba;

  lba = g_new(bytearray_t,1);
  bytearray_init(lba, len, 0, data);

  return lba;
}

gchar *delete_bytearray_keep_data(bytearray_t *ba) {
  gchar *ldata;
  ldata = (gchar *)ba->data;
  free(ba);
  return ldata;
}
