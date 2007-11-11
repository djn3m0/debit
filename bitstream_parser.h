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

#ifndef _BITSTREAM_PARSER_H
#define _BITSTREAM_PARSER_H

#include <stdint.h>
#include <glib.h>
#include "bitheader.h"

/* This structure holds the final results of the parsing phase.
   Actually it should be split in two, separating data (mapped files
   &al), from the rest.
 */
typedef struct _bitstream_parsed {
  /* First header informations */
  parsed_header_t header;

  /* (virtex-II) chip type info -- from bitstream data */
  const void *chip_struct;

  /* frames */
  const gchar ***frames;
  GArray *frame_array;

  /* mmapped file information */
  GMappedFile *file;

} bitstream_parsed_t;

int alloc_wbitstream(bitstream_parsed_t *bitstream);
void free_wbitstream(bitstream_parsed_t *parser);

bitstream_parsed_t *parse_bitstream(const gchar*filename);
void free_bitstream(bitstream_parsed_t *bitstream);

typedef void (*frame_iterator_t)(const char *frame,
				 guint type,
				 guint index,
				 guint frameidx,
				 void *data);

/****
 * Bitstream frame indexing
 ****/

void iterate_over_frames(const bitstream_parsed_t *parsed,
			 frame_iterator_t iter, void *data);

void
iterate_over_frames_far(const bitstream_parsed_t *parsed,
			frame_iterator_t iter, void *dat);

typedef struct _frame_record {
  guint32 far;
  guint32 offset;
  unsigned framelen;
  const char *frame;
} frame_record_t;

typedef void (*frame_unk_iterator_t)(const frame_record_t *frame,
				     void *data);

void iterate_over_unk_frames(const bitstream_parsed_t *parsed,
			     frame_unk_iterator_t iter, void *itdat);

/* for v2 */
void
typed_frame_name(char *buf, unsigned buf_len,
		 const unsigned type,
		 const unsigned index,
		 const unsigned frameid);

/* for v4, v5 */
int
snprintf_far(char *buf, const size_t buf_len,
	     const uint32_t hwfar);

#endif /* _BITSTREAM_PARSER_H */
