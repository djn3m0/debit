/*
 * Copyright (C) 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
 */

#ifndef _ALTERA_BITSTREAM_H
#define _ALTERA_BITSTREAM_H

#include <glib.h>
#include "bitarray.h"

typedef struct _altera_bitstream_t {
  bitarray_t *bitarray;
  unsigned *xoffsets;
  unsigned base_offset;

  const char *bitdata;
  guint32 bitlength;

  GMappedFile *file;
} altera_bitstream_t;

/* adjustable parameters */
extern gint base_off;
extern gint slice_off;

altera_bitstream_t *
parse_bitstream(const gchar *file);

void
dump_lut_tables(const altera_bitstream_t *);

int
dump_raw_bit(const gchar *odir, const gchar *filename,
	     const altera_bitstream_t *);
void
zero_lut_tables(const altera_bitstream_t *);

void
dump_lab_data(const gchar *odir,
	      const altera_bitstream_t *);

void free_bitstream(altera_bitstream_t *);

#endif /* _ALTERA_BITSTREAM_H */
