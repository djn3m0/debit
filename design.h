/*
 * (C) Copyright 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
 * All rights reserved.
 */

#include "bitarray.h"

#ifndef _DESIGN_H
#define _DESIGN_H


/* family specific */
typedef enum {
  V2C_IOB = 0,
  V2C_IOI,
  V2C_CLB,
  V2C_BRAM,
  V2C_BRAM_INT,
  V2C_GCLK,
  V2C__NB_CFG,
} v2_design_col_t;

/*
 * FAR Register implementation
 */

/*
 * FAR register -- hardware
 */

/* TODO: get rid of this. Higly non-portable */
typedef struct {
  unsigned int bn :9; // NOT in bytes, but in u32 words (or won't fit)
  unsigned int mna :8;
  unsigned int mja :8;
  unsigned int ba :2;
  unsigned int __rsvd :5;
} v2_frame_addr_t;

#define FAR_BN_OFFSET 0
#define FAR_BN_LEN 9
#define FAR_BN_MASK ((1<<FAR_BN_LEN) - 1) << FAR_BN_OFFSET

#define FAR_MNA_OFFSET (FAR_BN_OFFSET + FAR_BN_LEN)
#define FAR_MNA_LEN 8
#define FAR_MNA_MASK ((1<<FAR_MNA_LEN) - 1) << FAR_MNA_OFFSET

#define FAR_MJA_OFFSET (FAR_MNA_OFFSET + FAR_MNA_LEN)
#define FAR_MJA_LEN 8
#define FAR_MJA_MASK ((1<<FAR_MJA_LEN) - 1) << FAR_MJA_OFFSET

#define FAR_BA_OFFSET (FAR_MJA_OFFSET + FAR_MJA_LEN)
#define FAR_BA_LEN 2
#define FAR_BA_MASK ((1<<FAR_BA_LEN) - 1) << FAR_BA_OFFSET

static inline unsigned
bn_of_far(const uint32_t far) {
	return (far & FAR_BN_MASK) >> FAR_BN_OFFSET;
}

static inline unsigned
mna_of_far(const uint32_t far) {
	return (far & FAR_MNA_MASK) >> FAR_MNA_OFFSET;
}

static inline unsigned
mja_of_far(const uint32_t far) {
	return (far & FAR_MJA_MASK) >> FAR_MJA_OFFSET;
}

static inline unsigned
ba_of_far(const uint32_t far) {
	return (far & FAR_BA_MASK) >> FAR_BA_OFFSET;
}

/*
 * FAR register -- our more simple software vision of it
 */

typedef struct sw_far {
	unsigned bn;
	unsigned mna;
	unsigned mja;
	unsigned ba;
} sw_far_t;

static inline void
fill_swfar(sw_far_t *sw_far, const uint32_t hwfar) {
	sw_far->bn = bn_of_far(hwfar);
	sw_far->mna = mna_of_far(hwfar);
	sw_far->mja = mja_of_far(hwfar);
	sw_far->ba = ba_of_far(hwfar);
}

static inline uint32_t
get_hwfar(const sw_far_t *sw_far) {
	return (sw_far->bn +
		(sw_far->mna << FAR_MNA_OFFSET) +
		(sw_far->mja << FAR_MJA_OFFSET) +
		(sw_far->ba  << FAR_BA_OFFSET));
}

/*
 * V1 packet type
 */

typedef struct {
  unsigned word_count :11;
  unsigned __rsvd :2;
  unsigned reg_addr :14;
  unsigned rd :1;
  unsigned wr :1;
  unsigned type :3;
} v2_packet1_t;

#define V1_PKT_WORDC_OFFSET 0
#define V1_PKT_WORDC_LEN 11
#define V1_PKT_WORDC_MASK ((1<<V1_PKT_WORDC_LEN) - 1) << V1_PKT_WORDC_OFFSET

#define V1_PKT_RSVD_OFFSET (V1_PKT_WORDC_OFFSET + V1_PKT_WORDC_LEN)
#define V1_PKT_RSVD_LEN 2
#define V1_PKT_RSVD_MASK ((1<<V1_PKT_RSVD_LEN) - 1) << V1_PKT_RSVD_OFFSET

#define V1_PKT_REGA_OFFSET (V1_PKT_RSVD_OFFSET + V1_PKT_RSVD_LEN)
#define V1_PKT_REGA_LEN 14
#define V1_PKT_REGA_MASK ((1<<V1_PKT_REGA_LEN) - 1) << V1_PKT_REGA_OFFSET

#define V1_PKT_RD_OFFSET (V1_PKT_REGA_OFFSET + V1_PKT_REGA_LEN)
#define V1_PKT_RD_LEN 1
#define V1_PKT_RD_MASK ((1<<V1_PKT_RD_LEN) - 1) << V1_PKT_RD_OFFSET

#define V1_PKT_WR_OFFSET (V1_PKT_RD_OFFSET + V1_PKT_RD_LEN)
#define V1_PKT_WR_LEN 1
#define V1_PKT_WR_MASK ((1<<V1_PKT_WR_LEN) - 1) << V1_PKT_WR_OFFSET

#define V1_PKT_TYPE_OFFSET (V1_PKT_WR_OFFSET + V1_PKT_WR_LEN)
#define V1_PKT_TYPE_LEN 3
#define V1_PKT_TYPE_MASK ((1<<V1_PKT_TYPE_LEN) - 1) << V1_PKT_TYPE_OFFSET

static inline unsigned
wordc_of_pkt1(const uint32_t pkt1) {
  return (pkt1 & V1_PKT_WORDC_MASK) >> V1_PKT_WORDC_OFFSET;
}

static inline unsigned
rsvd_of_pkt1(const uint32_t pkt1) {
  return (pkt1 & V1_PKT_RSVD_MASK) >> V1_PKT_RSVD_OFFSET;
}

static inline unsigned
rega_of_pkt1(const uint32_t pkt1) {
  return (pkt1 & V1_PKT_REGA_MASK) >> V1_PKT_REGA_OFFSET;
}

static inline unsigned
rd_of_pkt1(const uint32_t pkt1) {
  return (pkt1 & V1_PKT_RD_MASK) >> V1_PKT_RD_OFFSET;
}

static inline unsigned
wr_of_pkt1(const uint32_t pkt1) {
  return (pkt1 & V1_PKT_WR_MASK) >> V1_PKT_WR_OFFSET;
}

static inline unsigned
type_of_pkt1(const uint32_t pkt1) {
  return (pkt1 & V1_PKT_TYPE_MASK) >> V1_PKT_TYPE_OFFSET;
}

/*
 * V2 packet type
 */
typedef struct {
  unsigned word_count :27;
  unsigned rd :1;
  unsigned wr :1;
  unsigned type :3;
} v2_packet2_t;

#define V2_PKT_WORDC_OFFSET 0
#define V2_PKT_WORDC_LEN 27
#define V2_PKT_WORDC_MASK ((1<<V2_PKT_WORDC_LEN) - 1) << V2_PKT_WORDC_OFFSET

#define V2_PKT_WR_OFFSET (V2_PKT_WORDC_OFFSET + V2_PKT_WORDC_LEN)
#define V2_PKT_WR_LEN 1
#define V2_PKT_WR_MASK ((1<<V2_PKT_WR_LEN) - 1) << V2_PKT_WR_OFFSET

#define V2_PKT_RD_OFFSET (V2_PKT_WR_OFFSET + V2_PKT_WR_LEN)
#define V2_PKT_RD_LEN 1
#define V2_PKT_RD_MASK ((1<<V2_PKT_RD_LEN) - 1) << V2_PKT_RD_OFFSET

#define V2_PKT_TYPE_OFFSET (V2_PKT_RD_OFFSET + V2_PKT_RD_LEN)
#define V2_PKT_TYPE_LEN 3
#define V2_PKT_TYPE_MASK ((1<<V2_PKT_TYPE_LEN) - 1) << V2_PKT_TYPE_OFFSET

static inline unsigned
wordc_of_v2pkt(const uint32_t v2pkt) {
  return (v2pkt & V2_PKT_WORDC_MASK) >> V2_PKT_WORDC_OFFSET;
}

static inline unsigned
rd_of_v2pkt(const uint32_t v2pkt) {
  return (v2pkt & V2_PKT_RD_MASK) >> V2_PKT_RD_OFFSET;
}

static inline unsigned
wr_of_v2pkt(const uint32_t v2pkt) {
  return (v2pkt & V2_PKT_WR_MASK) >> V2_PKT_WR_OFFSET;
}

static inline unsigned
type_of_v2pkt(const uint32_t v2pkt) {
  return (v2pkt & V2_PKT_TYPE_MASK) >> V2_PKT_TYPE_OFFSET;
}

/*
 * Other structures
 */

typedef uint32_t reg_t;

#define FAMILY_LEN 128

typedef struct design {
  /* for now be simple */
  unsigned char ***bincols;
  int frame_len;
  char family[FAMILY_LEN];
} design_t;

design_t *new_design(char *platform, char* family);
void delete_design(design_t *D);

void v2_design_writeback_frame(design_t *D, sw_far_t *sw_far, reg_t *frame);
void v2_swfar_increment(design_t *D, sw_far_t *addr);

static const
int v2_col_count[V2C__NB_CFG] = {
 [V2C_IOB] = 2,
 [V2C_IOI] = 2,
 [V2C_CLB] = 48,
 [V2C_BRAM] = 4,
 [V2C_BRAM_INT] =  4,
 [V2C_GCLK] = 1,
};

static const
int v2_frame_count[V2C__NB_CFG] = {
 [V2C_IOB] = 4,
 [V2C_IOI] = 22,
 [V2C_CLB] = 22,
 [V2C_BRAM] = 64,
 [V2C_BRAM_INT] =  22,
 [V2C_GCLK] = 4,
};

/* thoughts for later */

/*-er
  We try to use the following abstraction:
  _ we have units of configuration on the chip;
  _ we have a few different types of units, each type can be
  handled only knowing its position on the chip.
 */
//typedef void (*unit_handler_t)(unit_t *, unit_type_t *);

/*-er
  The concept is that cols describe the state of several
  logic_units (those could be of different kinds).
  Hopefully, two different kinds of cols do not overlap in
  in the properties of the design they control.
  I just hope the chip vendors are clever.
  logic_units could be hierarchical. Should they?
*/

/*
typedef enum {
  // controlled by CLB
  // general p22
  // distributed RAM p25
  // SRL p28
  V2L_LUT,
  V2L_,
  V2L_,
  // controlled by IOI
  V2L_,
  // controlled by IOB - p37(input) p41(output) p44(3-states)
  V2L_,
  // controlled by GCLK - p61
  // controlled by BRAM
  // controlled by BRAM_INT
} v2_design_logic_units;
*/

#endif /* design.h */
