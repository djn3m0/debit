/*
 * (C) Copyright 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
 * All rights reserved.
 */

#ifndef _FILEDUMP_H
#define _FILEDUMP_H

#include <glib.h>
#include "bitstream_parser.h"

void design_write_frames(const bitstream_parsed_t *parsed,
			 const gchar *outdir);

void design_dump_frames(const bitstream_parsed_t *parsed,
			const gchar *outdir);

#endif /* filedump.h */
