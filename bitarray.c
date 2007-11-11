/*
 * Copyright (C) 2006, 2007 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
 *
 * This file is part of debit.
 *
 * Debit is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Debit is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with debit.  If not, see <http://www.gnu.org/licenses/>.
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
