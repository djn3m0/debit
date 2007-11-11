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

#ifndef _ALTERA_BITSTREAM_H
#define _ALTERA_BITSTREAM_H

#include <stdint.h>
#include <glib.h>
#include "bitarray.h"

typedef struct _altera_bitstream_t {
  bitarray_t *bitarray;
  unsigned *xoffsets;
  unsigned base_offset;

  /* bitstream, pips & al */
  const char *bitdata;
  guint32 bitlength;

  /* M4k ramblocks of the cyclone 2. At least part of it */
  const char *m4kdata;
  guint32 m4klength;

  GMappedFile *file;
} altera_bitstream_t;

/* adjustable parameters */
extern gint base_off;
extern gint slice_off;

altera_bitstream_t *
parse_bitstream(const gchar *file);

void
dump_lut_tables(const altera_bitstream_t *);

/* TODO: factor this into a "dumpoption" thing */

int
dump_raw_bit(const gchar *odir, const gchar *filename,
	     const altera_bitstream_t *);
int
dump_raw_m4k(const gchar *odir, const gchar *filename,
	     const altera_bitstream_t *);

void
zero_lut_tables(const altera_bitstream_t *);

void
dump_lab_data(const gchar *odir,
	      const altera_bitstream_t *);

void
print_pos_from_bit_offset(const altera_bitstream_t *,
			  unsigned offset);

void free_bitstream(altera_bitstream_t *);

#endif /* _ALTERA_BITSTREAM_H */
