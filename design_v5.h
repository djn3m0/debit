/*
 * (C) Copyright 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
 * All rights reserved.
 */

#ifndef _DESIGN_V5_H
#define _DESIGN_V5_H

#include <stdint.h>

/*
 * FAR Register implementation
 */

/*
 * FAR register -- hardware
 */

/* TODO: get rid of this. Higly non-portable */
typedef struct {
  unsigned int mna :7;
  unsigned int col :8;
  unsigned int row :5;
  unsigned int tb :1;
  unsigned int type :3;
} v5_frame_addr_t;

#define FAR_V5_MNA_OFFSET 0
#define FAR_V5_MNA_LEN 7
#define FAR_V5_MNA_MASK ((1<<FAR_V5_MNA_LEN) - 1) << FAR_V5_MNA_OFFSET

#define FAR_V5_COL_OFFSET (FAR_V5_MNA_OFFSET + FAR_V5_MNA_LEN)
#define FAR_V5_COL_LEN 8
#define FAR_V5_COL_MASK ((1<<FAR_V5_COL_LEN) - 1) << FAR_V5_COL_OFFSET

#define FAR_V5_ROW_OFFSET (FAR_V5_COL_OFFSET + FAR_V5_COL_LEN)
#define FAR_V5_ROW_LEN 5
#define FAR_V5_ROW_MASK ((1<<FAR_V5_ROW_LEN) - 1) << FAR_V5_ROW_OFFSET

#define FAR_V5_TB_OFFSET (FAR_V5_ROW_OFFSET + FAR_V5_ROW_LEN)
#define FAR_V5_TB_LEN 1
#define FAR_V5_TB_MASK ((1<<FAR_V5_TB_LEN) - 1) << FAR_V5_TB_OFFSET

#define FAR_V5_TYPE_OFFSET (FAR_V5_TB_OFFSET + FAR_V5_TB_LEN)
#define FAR_V5_TYPE_LEN 3
#define FAR_V5_TYPE_MASK ((1<<FAR_V5_TYPE_LEN) - 1) << FAR_V5_TYPE_OFFSET

static inline unsigned
v5_mna_of_far(const uint32_t far) {
	return (far & FAR_V5_MNA_MASK) >> FAR_V5_MNA_OFFSET;
}

static inline unsigned
v5_col_of_far(const uint32_t far) {
	return (far & FAR_V5_COL_MASK) >> FAR_V5_COL_OFFSET;
}

static inline unsigned
v5_row_of_far(const uint32_t far) {
	return (far & FAR_V5_ROW_MASK) >> FAR_V5_ROW_OFFSET;
}

static inline unsigned
v5_tb_of_far(const uint32_t far) {
	return (far & FAR_V5_TB_MASK) >> FAR_V5_TB_OFFSET;
}

static inline unsigned
v5_type_of_far(const uint32_t far) {
	return (far & FAR_V5_TYPE_MASK) >> FAR_V5_TYPE_OFFSET;
}

/*
 * FAR register -- our more simple software vision of it
 */

typedef struct sw_far_v5 {
  unsigned tb;
  unsigned type;
  unsigned row;
  unsigned col;
  unsigned mna;
} sw_far_v5_t;

static inline void
fill_swfar_v5(sw_far_v5_t *sw_far, const uint32_t hwfar) {
  sw_far->tb = v5_tb_of_far(hwfar);
  sw_far->type = v5_type_of_far(hwfar);
  sw_far->row = v5_row_of_far(hwfar);
  sw_far->col = v5_col_of_far(hwfar);
  sw_far->mna = v5_mna_of_far(hwfar);
}

static inline uint32_t
get_hwfar_v5(const sw_far_v5_t *sw_far) {
  return (sw_far->mna +
	  (sw_far->col << FAR_V5_COL_OFFSET) +
	  (sw_far->row << FAR_V5_ROW_OFFSET) +
	  (sw_far->type << FAR_V5_TYPE_OFFSET) +
	  (sw_far->tb << FAR_V5_TB_OFFSET));
}

#include <stdio.h>

static inline int
snprintf_far_v5(char *buf, const size_t buf_len,
		const uint32_t hwfar) {
  return snprintf(buf, buf_len,
		  "%i_%i_%i_%i_%i",
		  v5_tb_of_far(hwfar),
		  v5_type_of_far(hwfar),
		  v5_row_of_far(hwfar),
		  v5_col_of_far(hwfar),
		  v5_mna_of_far(hwfar));
}

#endif /* design_v5.h */
