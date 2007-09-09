/*
 * (C) Copyright 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
 * All rights reserved.
 *
 */

#ifndef _BITSTREAM_PARSER_H
#define _BITSTREAM_PARSER_H

#include <stdint.h>
#include <glib.h>
#include "bitheader.h"

/* This structure holds the finale results of the parsing phase */
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
