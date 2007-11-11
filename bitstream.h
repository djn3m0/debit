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

#ifndef _BITSTREAM_H
#define _BITSTREAM_H

#include "bitstream_parser.h"
#include "sites.h"

/** \file
 *
 * Interface file for accessing the bitstream data in a
 * pseudo-structured way.
 */

void
query_bitstream_luts(const bitstream_parsed_t *, const csite_descr_t *, guint16[]);
void
set_bitstream_lut(const bitstream_parsed_t *bitstream,
		  const csite_descr_t *site,
		  const guint16 lut_val, const unsigned lut_i);

guint32
query_bitstream_site_bits(const bitstream_parsed_t *, const csite_descr_t *,
			  const guint *, const gsize);

void
set_bitstream_site_bits(const bitstream_parsed_t *, const csite_descr_t *,
			const uint32_t vals, const guint cfgbits[], const gsize nbits);

guint16 *
query_bitstream_bram_data(const bitstream_parsed_t *bitstream, const csite_descr_t *site);

gsize
query_bitstream_type_size(const bitstream_parsed_t *parsed,
			  const site_type_t type);

int
query_bitstream_site_data(gchar *data, const gsize nbytes,
	                  const bitstream_parsed_t *bitstream,
			  const csite_descr_t *site);

#endif /* _BITSTREAM_H */
