/*
 * (C) Copyright 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
 * All rights reserved.
 */

#ifndef _DESIGN_V4_H
#define _DESIGN_V4_H

#include <stdint.h>
#include "bitstream_parser.h"

/*
 * FAR Register implementation
 */

/*
 * FAR register -- hardware
 */

/* this is only as a remembrance of things past.
   Higly non-portable, so don't you dare use it. */
typedef struct {
  unsigned int mna :6;
  unsigned int col :8;
  unsigned int row :5;
  unsigned int type :3;
  unsigned int tb :1;
} v4_frame_addr_t;

#define FAR_V4_MNA_OFFSET 0
#define FAR_V4_MNA_LEN 6
#define FAR_V4_MNA_MASK ((1<<FAR_V4_MNA_LEN) - 1) << FAR_V4_MNA_OFFSET

#define FAR_V4_COL_OFFSET (FAR_V4_MNA_OFFSET + FAR_V4_MNA_LEN)
#define FAR_V4_COL_LEN 8
#define FAR_V4_COL_MASK ((1<<FAR_V4_COL_LEN) - 1) << FAR_V4_COL_OFFSET

#define FAR_V4_ROW_OFFSET (FAR_V4_COL_OFFSET + FAR_V4_COL_LEN)
#define FAR_V4_ROW_LEN 5
#define FAR_V4_ROW_MASK ((1<<FAR_V4_ROW_LEN) - 1) << FAR_V4_ROW_OFFSET

#define FAR_V4_TYPE_OFFSET (FAR_V4_ROW_OFFSET + FAR_V4_ROW_LEN)
#define FAR_V4_TYPE_LEN 3
#define FAR_V4_TYPE_MASK ((1<<FAR_V4_TYPE_LEN) - 1) << FAR_V4_TYPE_OFFSET

#define FAR_V4_TB_OFFSET (FAR_V4_TYPE_OFFSET + FAR_V4_TYPE_LEN)
#define FAR_V4_TB_LEN 1
#define FAR_V4_TB_MASK ((1<<FAR_V4_TB_LEN) - 1) << FAR_V4_TB_OFFSET

typedef enum _v4_col_type {
  V4_TYPE_CLB = 0,
  V4_TYPE_BRAM_INT,
  V4_TYPE_BRAM,
  V4_TYPE_CFG_CLB,
  V4_TYPE_CFG_BRAM,
  V4__NB_COL_TYPES,
} v4_col_type_t;

typedef enum _v4_tb_t {
  V4_TB_TOP = 0,
  V4_TB_BOTTOM,
} v4_tb_t;

static inline unsigned
v4_mna_of_far(const uint32_t far) {
	return (far & FAR_V4_MNA_MASK) >> FAR_V4_MNA_OFFSET;
}

static inline unsigned
v4_col_of_far(const uint32_t far) {
	return (far & FAR_V4_COL_MASK) >> FAR_V4_COL_OFFSET;
}

static inline unsigned
v4_row_of_far(const uint32_t far) {
	return (far & FAR_V4_ROW_MASK) >> FAR_V4_ROW_OFFSET;
}

static inline v4_col_type_t
v4_type_of_far(const uint32_t far) {
	return (far & FAR_V4_TYPE_MASK) >> FAR_V4_TYPE_OFFSET;
}

static inline v4_tb_t
v4_tb_of_far(const uint32_t far) {
	return (far & FAR_V4_TB_MASK) >> FAR_V4_TB_OFFSET;
}

/*
 * FAR register -- our more simple software vision of it
 */

typedef struct sw_far_v4 {
  v4_tb_t tb;
  v4_col_type_t type;
  unsigned row;
  unsigned col;
  unsigned mna;
} sw_far_v4_t;

static inline void
fill_swfar_v4(sw_far_v4_t *sw_far, const uint32_t hwfar) {
  sw_far->tb = v4_tb_of_far(hwfar);
  sw_far->type = v4_type_of_far(hwfar);
  sw_far->row = v4_row_of_far(hwfar);
  sw_far->col = v4_col_of_far(hwfar);
  sw_far->mna = v4_mna_of_far(hwfar);
}

static inline uint32_t
get_hwfar_v4(const sw_far_v4_t *sw_far) {
  return (sw_far->mna +
	  (sw_far->col << FAR_V4_COL_OFFSET) +
	  (sw_far->row << FAR_V4_ROW_OFFSET) +
	  (sw_far->type << FAR_V4_TYPE_OFFSET) +
	  (sw_far->tb << FAR_V4_TB_OFFSET));
}

#include <stdio.h>

static inline int
snprintf_far_v4(char *buf, const size_t buf_len,
		const uint32_t hwfar) {
  return snprintf(buf, buf_len,
		  "%i_%i_%i_%i_%i",
		  v4_tb_of_far(hwfar),
		  v4_type_of_far(hwfar),
		  v4_row_of_far(hwfar),
		  v4_col_of_far(hwfar),
		  v4_mna_of_far(hwfar));
}

typedef enum {
  V4C_IOB = 0,
  V4C_CLB,
  V4C_DSP48,
  V4C_GCLK,
  V4C_BRAM,
  V4C_BRAM_INT,
  V4C_PAD,
  V4C__NB_CFG,
} v4_design_col_t;

typedef struct _chip_struct {
  guint32 idcode;
  guint32 framelen;
  const int *frame_count;
  const unsigned col_count[V4__NB_COL_TYPES];
  const unsigned row_count;
} chip_struct_t;

static inline void
typed_frame_name(char *buf, unsigned buf_len,
		 const unsigned type,
		 const unsigned index,
		 const unsigned frameid) {
}

/**** Frame fast indexing ****/

static inline unsigned
type_col_count(const unsigned *col_count,
	       const v4_design_col_t type) {
  switch (type) {
  case V4C_IOB:
    return 3;
  case V4C_GCLK:
    return 1;
  case V4C_DSP48:
    return 1;
  case V4C_CLB:
    return col_count[V4_TYPE_CLB] - 5;
  case V4C_BRAM:
    return col_count[V4_TYPE_BRAM];
  case V4C_BRAM_INT:
    return col_count[V4_TYPE_BRAM_INT];
  case V4C_PAD:
    return V4__NB_COL_TYPES;
  case V4C__NB_CFG:
    /* return the total ? */
  default:
    g_assert_not_reached();
  }
}

/*
 * The frame index is a four-way lookup table. We have chosen for now to use
 * a two-way lookup table to index the frames internally, redoing most
 * of the computation for the v4.
 */

static inline
const gchar **get_frame_loc(const bitstream_parsed_t *parsed,
			    const guint type,
			    const guint row,
			    const guint top,
			    const guint index,
			    const guint frame) {
  const chip_struct_t *chip_struct = parsed->chip_struct;
  const unsigned rowcount = chip_struct->row_count;
  const unsigned framecount = chip_struct->frame_count[type];
  const unsigned *col_count = chip_struct->col_count;
  g_assert(type < V4C__NB_CFG);
  //  g_assert(index < type_col_count(col_count, type));
  if ( index >= type_col_count(col_count, type))
    g_warning("problem in index %i >= %i for type %i", index, type_col_count(col_count, type), type);
  g_assert(row < rowcount);
  //  g_assert(frame < framecount);
  if (frame >= framecount)
    g_warning("problem in frame %i >= %i for type %i", frame, framecount, type);
  (void) col_count;

  /* This is a double-lookup method */
  return &parsed->frames[type][ index * (2 * rowcount * framecount) +
				(top * rowcount + row) * framecount +
				frame ];
}

static inline
const gchar *get_frame(const bitstream_parsed_t *parsed,
		       const guint type,
		       const guint row,
		       const guint top,
		       const guint index,
		       const guint frame) {
  const gchar *frameptr = *get_frame_loc(parsed, type, row, top, index, frame);
  g_assert(frameptr != NULL);
  return frameptr;
}

#endif /* design_v4.h */
