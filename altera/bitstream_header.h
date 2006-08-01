/*
 * Copyright (C) 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
 */

#ifndef _BITSTREAM_HEADER_H
#define _BITSTREAM_HEADER_H

#include "bitstream.h"

/* first-level, gross parsing of the bitstream */
int parse_bitstream_structure(altera_bitstream_t *,
			      const gchar *, const size_t);

#endif /* _BITSTREAM_HEADER_H */
