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

void dump_sites(const bitstream_analyzed_t *nlz,
		const gchar *odir, const gchar *suffix);

void dump_pips(bitstream_analyzed_t *bitstream);
void dump_luts(bitstream_analyzed_t *bitstream);
void dump_bram(bitstream_analyzed_t *bitstream);
void dump_nets(const bitstream_analyzed_t *bitstream);

#endif /* _HAS_ANALYSIS_H */
