/*
 * (C) Copyright 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
 * All rights reserved.
 *
 */

#ifndef _BITSTREAM_PARSER_H
#define _BITSTREAM_PARSER_H

#include <glib.h>
#include "design.h"

typedef enum _id {
  XC2V40 = 0, XC2V80,
  XC2V250, XC2V500,
  XC2V1000, XC2V1500,
  XC2V2000, XC2V3000,
  XC2V4000, XC2V6000,
  XC2V8000,
  XC2__NUM,
} id_t;

/* This structure holds the finale results of the parsing phase */
typedef struct _bitstream_parsed {
  /* First header informations */
  gchar *filename;
  gchar *device;
  gchar *build_date;
  gchar *build_time;

  /* (virtex-II) chip type info -- from bitstream data */
  id_t chip;
  const chip_struct_t *chip_struct;

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
  const chip_struct_t *chip_struct = parsed->chip_struct;
  const int *col_count = chip_struct->col_count;
  const int *frame_count = chip_struct->frame_count;
  g_assert(type < V2C__NB_CFG);
  g_assert(index < col_count[type]);
  g_assert(frame < frame_count[type]);

  /* This is a double-lookup method */
  return &parsed->frames[type][index * frame_count[type] + frame];
}

/* FDRI handling. Requires FAR handling.
   Registers a frame */

static inline
const gchar *get_frame(const bitstream_parsed_t *parsed,
		       const guint type,
		       const guint index,
		       const guint frame) {
  const gchar *frameptr = *get_frame_loc(parsed, type, index, frame);
  g_assert(frameptr != NULL);
  return frameptr;
}

#endif /* _BITSTREAM_PARSER_H */
