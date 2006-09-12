/*
 * (C) Copyright 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
 * All rights reserved.
 */

#ifndef _HAS_CONNEXITY_H
#define _HAS_CONNEXITY_H

#include <glib.h>
#include "wiring.h"
#include "localpips.h"

typedef struct _bitstream_analyzed {
  /* low-level information */
  bitstream_parsed_t *bitstream;

  /* databases */
  chip_descr_t *chip;
  pip_db_t *pipdb;

  /* nets from the bitstream */

} bitstream_analyzed_t;

void free_analysis(bitstream_analyzed_t *anal);

bitstream_analyzed_t *
analyze_bitstream(bitstream_parsed_t *bitstream,
		  const gchar *datadir);

void
dump_pips(bitstream_analyzed_t *bitstream);

#endif /* _HAS_CONNEXITY_H */
