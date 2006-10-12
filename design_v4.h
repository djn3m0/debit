/*
 * (C) Copyright 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
 * All rights reserved.
 */

#ifndef _DESIGN_V4_H
#define _DESIGN_V4_H

/*
 * FAR Register implementation
 */

/*
 * FAR register -- hardware
 */

/* TODO: get rid of this. Higly non-portable */
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

static inline unsigned
v4_type_of_far(const uint32_t far) {
	return (far & FAR_V4_TYPE_MASK) >> FAR_V4_TYPE_OFFSET;
}

static inline unsigned
v4_tb_of_far(const uint32_t far) {
	return (far & FAR_V4_TB_MASK) >> FAR_V4_TB_OFFSET;
}

/*
 * FAR register -- our more simple software vision of it
 */

typedef struct sw_far_v4 {
  unsigned tb;
  unsigned type;
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

#endif /* design_v4.h */
