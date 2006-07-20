/*
 * (C) Copyright 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
 * All rights reserved.
 *
 */

#ifndef _BITSTREAM_PARSER_H
#define _BITSTREAM_PARSER_H

#include <glib.h>
#include "design.h"

/* This structure holds the finale results of the parsing phase */
typedef struct _bitstream_parsed {
  /* First header informations */
  gchar *filename;
  gchar *device;
  gchar *build_date;
  gchar *build_time;

  /* (virtex-II) chip type info -- from bitstream data */

  /* frames */
  gint frame_len;
  const gchar ***frames;

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

void iterate_over_frames(const bitstream_parsed_t *parsed,
			 frame_iterator_t iter, void *data);

/****
 * Bitstream frame indexing
 ****/

/*
 * The frame index is a three-way lookup table. We choose for now to use
 * a two-way lookup table to index the frames internally.
 */

static inline
const gchar **get_frame_loc(const bitstream_parsed_t *parsed,
			    const guint type,
			    const guint index,
			    const guint frame) {
  g_assert(type < V2C__NB_CFG);
  g_assert(index < v2_col_count[type]);
  g_assert(frame < v2_frame_count[type]);

  /* This is a double-lookup method */
  return &parsed->frames[type][index * v2_frame_count[type] + frame];
}

/* FDRI handling. Requires FAR handling.
   Registers a frame */

static inline
const gchar *get_frame(const bitstream_parsed_t *parsed,
		       const guint type,
		       const guint index,
		       const guint frame) {
  return *get_frame_loc(parsed, type, index, frame);
}

#endif /* _BITSTREAM_PARSER_H */
