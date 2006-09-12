/*
 * (C) Copyright 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
 * All rights reserved.
 *
 */

#ifndef _BITSTREAM_H
#define _BITSTREAM_H

#include "bitstream_parser.h"
#include "wiring.h"
#include "sites.h"

/** \file
 *
 * Interface file for accessing the bitstream data in a
 * pseudo-structured way.
 */

/** bitstream opaque type
 *
 * This is an abstract view of a bitstream. For now this is identical to
 * design_t, but I'm not sure it should be in the long term, so keep this
 * difference in name until we're settled.
 */

void
query_bitstream_luts(const bitstream_parsed_t *, const csite_descr_t *, guint16[]);

guint32
query_bitstream_site_bits(const bitstream_parsed_t *, const csite_descr_t *,
			  const guint *, const gsize);
guint16 *
query_bitstream_bram_data(const bitstream_parsed_t *bitstream, const csite_descr_t *site);

#endif /* _BITSTREAM_H */
