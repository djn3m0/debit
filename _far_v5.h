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

#include "bitstream_v5.h"

static const unsigned
v5_frame_count[V5C__NB_CFG] = {
  [V5C_IOB] = 54,
  [V5C_CLB] = 36,
  [V5C_DSP48] = 28,
  [V5C_GCLK] = 4,
  [V5C_BRAM_INT] = 30,
  [V5C_BRAM] = 128,
  [V5C_PAD] = 2,
};

static const
chip_struct_t bitdescr[XC5VLX__NUM] = {
  /* FLR is always 41 for virtex-5 */
  [XC5VLX30] = { .idcode = 0x286E093,
		 .chip = XC5VLX30,
		 .framelen = 41,
		 .frame_count = v5_frame_count,
		 .col_count = {
		   [V5_TYPE_CLB] = XC5VLX30_NFRAMES,
		   [V5_TYPE_BRAM] = XC5VLX30_NBRAMS,
		 },
		 .col_type = xc5vlx30,
		 .row_count = XC5VLX30_NROWS,
  },
  [XC5VLX50] = { .idcode = 0x2896093,
		 .chip = XC5VLX50,
		 .framelen = 41,
		 .frame_count = v5_frame_count,
		 .col_count = {
		   [V5_TYPE_CLB] = XC5VLX30_NFRAMES,
		   [V5_TYPE_BRAM] = XC5VLX30_NBRAMS,
		 },
		 .col_type = xc5vlx30,
		 .row_count = XC5VLX30_NROWS+1,
  },
  [XC5VLX85] = { .idcode = 0x28AE093,
		 .chip = XC5VLX85,
		 .framelen = 41,
		 .frame_count = v5_frame_count,
		 .col_count = {
		   [V5_TYPE_CLB] = XC5VLX85_NFRAMES,
		   [V5_TYPE_BRAM] = XC5VLX85_NBRAMS,
		 },
		 .col_type = xc5vlx85,
		 .row_count = XC5VLX85_NROWS,
  },
  [XC5VLX110] = { .idcode = 0x28D6093,
		  .chip = XC5VLX110,
		  .framelen = 41,
		  .frame_count = v5_frame_count,
		  .col_count = {
		    [V5_TYPE_CLB] = XC5VLX85_NFRAMES,
		    [V5_TYPE_BRAM] = XC5VLX85_NBRAMS,
		  },
		  .col_type = xc5vlx85,
		  .row_count = XC5VLX85_NROWS + 1,
  },
  [XC5VLX220] = { .idcode = 0x290C093,
		  .chip = XC5VLX220,
		  .framelen = 41,
		  .frame_count = v5_frame_count,
		  .col_count = {
		    [V5_TYPE_CLB] = XC5VLX220_NFRAMES,
		    [V5_TYPE_BRAM] = XC5VLX220_NBRAMS,
		  },
		  .col_type = xc5vlx220,
		  .row_count = XC5VLX220_NROWS,
  },
  /* not verified, I cannot synthetize for this chip */
  [XC5VLX330] = { .idcode = 0x295C093,
		  .chip = XC5VLX330,
		  .framelen = 41,
		  .frame_count = v5_frame_count,
		  .col_count = {
		    [V5_TYPE_CLB] = XC5VLX220_NFRAMES,
		    [V5_TYPE_BRAM] = XC5VLX220_NBRAMS,
		  },
		  .col_type = xc5vlx220,
		  .row_count = XC5VLX220_NROWS + 1,
  },
};

typedef enum _ba_v5_col_type {
  BA_TYPE_CLB = 0,
  /* Error in UG191, I guess */
  BA_TYPE_BRAM_INT,
  BA_TYPE_BRAM,
} ba_v5_col_type_t;

typedef enum parser_state {
  STATE_IDLE = 0,
  STATE_UNSYNCHED,
  STATE_WAITING_CTRL,
  STATE_WAITING_DATA,
} parser_state_t;

typedef enum _v5_registers_index {
  /* from xilinx ug071.pdf */
  CRC = 0, FAR, FDRI, FDRO,
  CMD, CTL0, MASK,
  STAT, LOUT, COR0,
  MFWR, CBC, IDCODE,
  AXSS, COR1, CSOB, WBSTAR,
  TIMER,
  BOOTSTS = 22,
  CTL1 = 24,
  __V5_NUM_REGISTERS,
} register_index_t;

typedef enum _cmd_code {
  CMD_NULL = 0,
  WCFG, MFW,
  DGHIGH_LFRM, RCFG, START, RCAP, RCRC,
  AGHIGH, SWITCH, GRESTORE,
  SHUTDOWN, GCAPTURE,
  DESYNCH, RESERVED0, IPROG,
  CRCC, LTIMER,
  __NUM_CMD_CODE,
} cmd_code_t;

#if DEBIT_DEBUG > 0
static const
char *reg_names[__V5_NUM_REGISTERS] = {
  [CRC] = "CRC",
  [FAR] = "FAR",
  [FDRI] = "FDRI",
  [FDRO] = "FDRO",
  [CMD] = "CMD",
  [CTL0] = "CTL0",
  [MASK] = "MASK",
  [STAT] = "STAT",
  [LOUT] = "LOUT",
  [COR0] = "COR0",
  [MFWR] = "MFWR",
  [CBC] = "CBC",
  [IDCODE] = "IDCODE",
  [AXSS] = "AXSS",
  [COR1] = "COR1",
  [CSOB] = "CSOB",
  [WBSTAR] = "WBSTAR",
  [TIMER] = "TIMER",
  [BOOTSTS] = "BOOTSTS",
  [CTL1] = "CTL1",
};

static const
char *cmd_names[__NUM_CMD_CODE] = {
  [CMD_NULL] = "NULL",
  [WCFG] = "WCFG",
  [MFW] = "MFWR",
  [DGHIGH_LFRM] = "LFRM",
  [RCFG] = "RCFG",
  [START] = "START",
  [RCAP] = "RCAP",
  [RCRC] = "RCRC",
  [AGHIGH] = "AGHIGH",
  [SWITCH] = "SWITCH",
  [GRESTORE] = "GRESTORE",
  [SHUTDOWN] = "SHUTDOWN",
  [GCAPTURE] = "GCAPTURE",
  [DESYNCH] = "DESYNCH",
};
#endif

#define sw_far_t sw_far_v5_t
#define id_vlx_t id_v5vlx_t
#define col_type_t v5_col_type_t
#define design_col_t v5_design_col_t
#define XC_VLX__NUM XC5VLX__NUM
#define VC__NB_CFG V5C__NB_CFG
#define get_hwfar get_hwfar_v5
#define fill_swfar fill_swfar_v5
#define frame_count_v v5_frame_count

/* This structure contains the internal structure of the parser */
typedef struct _bitstream_parser {
  /* state of the parser */
  parser_state_t state;
  /* state of the register engine */
  register_index_t active_register;
  gint write__read;
  gint active_length;

  /* Actual state of the various registers */
  xil_register_t registers[__V5_NUM_REGISTERS];

  /* detailed view of some registers */
  id_vlx_t type;

  /* Specific FDRI quirks */
  const void *last_frame;

  /* Bitstream proper */
  bytearray_t ba;
} bitstream_parser_t;

/*
 * Length, in words, of a frame.
 */

#define frame_length 41

/***
 * FAR handling
 */

static inline void _far_increment_type(sw_far_t *addr);
static inline gboolean _far_is_pad(const bitstream_parser_t *bitstream,
				   const sw_far_t *addr);

static inline void
_far_increment_tb(sw_far_t *addr) {
  v5_tb_t tb = addr->tb;
  switch (tb) {
  case V5_TB_TOP:
    addr->tb = V5_TB_BOTTOM;
    break;
  case V5_TB_BOTTOM:
    addr->tb = V5_TB_TOP;
    /* increase type */
    _far_increment_type(addr);
    break;
  default:
    g_assert_not_reached();
  }
}

int
snprintf_far(char *buf, const size_t buf_len,
	     const uint32_t hwfar) {
  return snprintf(buf, buf_len,
		  "%i_%i_%i_%i_%i",
		  v5_tb_of_far(hwfar),
		  v5_type_of_far(hwfar),
		  v5_row_of_far(hwfar),
		  v5_col_of_far(hwfar),
		  v5_mna_of_far(hwfar));
}

/* Type is a bit strange -- it watches the type from the far and the col
   count, so as to get a mixed type which depends on both these
   parameters. */

static inline design_col_t
_type_of_far(const bitstream_parser_t *bitstream,
	     const sw_far_t *addr) {
  const id_vlx_t chiptype = bitstream->type;
  const col_type_t type = addr->type;

  if (_far_is_pad(bitstream, addr))
    return V5C_PAD;

  switch (type) {
  case V5_TYPE_CLB: {
    const unsigned col = addr->col;
    return bitdescr[chiptype].col_type[col];
  }
  case V5_TYPE_BRAM:
    return V5C_BRAM;
  case V5_TYPE_CFG_CLB:
    g_print("Unknown frame type, please report your bitstream");
  default:
    g_assert_not_reached();
  }

  return -1;
}

static inline unsigned
typed_col_dumb(const design_col_t *array,
	       const unsigned col) {
  const design_col_t type = array[col];
  unsigned i, count = 0;

  for (i = 0; i < col; i++)
    if (array[i] == type)
      count++;

  return count;
}


static inline unsigned
_typed_col_of_far(const bitstream_parser_t *bitstream,
		  const sw_far_t *addr) {
  const id_vlx_t chiptype = bitstream->type;
  const col_type_t type = addr->type;
  const unsigned col = addr->col;

  /* unlikely, move this out of main exec flow */
  if (_far_is_pad(bitstream, addr))
    return type;

  switch (type) {
  case V5_TYPE_CLB: {
    const design_col_t *typear = bitdescr[chiptype].col_type;
    return typed_col_dumb(typear, col);
  }
  default:
    return col;
  }

  return -1;
}
