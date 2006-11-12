/*
 * (C) Copyright 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
 * All rights reserved.
 */

#ifndef _HAS_ANALYSIS_H
#define _HAS_ANALYSIS_H

#include <glib.h>
#include "wiring.h"
#include "localpips.h"

typedef struct _bitstream_analyzed {
  /* low-level information */
  bitstream_parsed_t *bitstream;

  /* databases */
  chip_descr_t *chip;
  pip_db_t *pipdb;

  /* simplified bitstream data */
  pip_parsed_dense_t *pipdat;

  /* nets from the bitstream */

} bitstream_analyzed_t;

void free_analysis(bitstream_analyzed_t *anal);

bitstream_analyzed_t *
analyze_bitstream(bitstream_parsed_t *bitstream,
		  const gchar *datadir);

void dump_sites(const bitstream_analyzed_t *nlz, const gchar *odir);

void dump_pips(bitstream_analyzed_t *bitstream);
void dump_luts(bitstream_analyzed_t *bitstream);
void dump_bram(bitstream_analyzed_t *bitstream);

#endif /* _HAS_ANALYSIS_H */
