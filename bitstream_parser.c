/*
 * (C) Copyright 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
 * All rights reserved.
 *
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <glib.h>

#include "bitarray.h"
#include "design.h"
#include "bitheader.h"
#include "bitstream_parser.h"
#include "bitstream_packets.h"
#include "debitlog.h"
#include "codes/crc-ibm.h"

/* This should be merged into the things below */
#if defined(VIRTEX2)

static const gchar *
chipfiles[XC2__NUM] = {
  [XC2V40] = "xc2v40",
  [XC2V80] = "xc2v80",
  [XC2V250] = "xc2v250",
  [XC2V500] = "xc2v500",
  [XC2V1000] = "xc2v1000",
  [XC2V1500] = "xc2v1500",
  [XC2V2000] = "xc2v2000",
  [XC2V3000] = "xc2v3000",
  [XC2V4000] = "xc2v4000",
  [XC2V6000] = "xc2v6000",
  [XC2V8000] = "xc2v8000",
};

#elif defined(SPARTAN3)

static const gchar *
chipfiles[XC3__NUM] = {
  [XC3S50] = "xc3s50",
  [XC3S200] = "xc3s200",
  [XC3S400] = "xc3s400",
  [XC3S1000] = "xc3s1000",
  [XC3S1500] = "xc3s1500",
  [XC3S2000] = "xc3s2000",
  [XC3S4000] = "xc3s4000",
  [XC3S5000] = "xc3s5000",
};

#endif

/* This is nothing but a key-value file */
#if defined(VIRTEX2)
static const
unsigned v2_frame_count[V2C__NB_CFG] = {
  [V2C_IOB] = 4,
  [V2C_IOI] = 22,
  [V2C_CLB] = 22,
  [V2C_BRAM] = 64,
  [V2C_BRAM_INT] = 22,
  [V2C_GCLK] = 4,
};

static const
chip_struct_t bitdescr[XC2__NUM] = {
  [XC2V40] = {
    .chip = XC2V40,
    .idcode = 0x01008093U,
    .framelen = 26,
    .col_count = {
      [V2C_IOB] = 2,
      [V2C_IOI] = 2,
      [V2C_CLB] = 8,
      [V2C_BRAM] = 2,
      [V2C_BRAM_INT] = 2,
      [V2C_GCLK] = 1,
    },
    .frame_count = v2_frame_count,
  },
  [XC2V80] = {
    .chip = XC2V80,
    .idcode = 0x01010093U,
    .framelen = 46,
    .col_count = {
      [V2C_IOB] = 2,
      [V2C_IOI] = 2,
      [V2C_CLB] = 8,
      [V2C_BRAM] = 2,
      [V2C_BRAM_INT] = 2,
      [V2C_GCLK] = 1,
    },
    .frame_count = v2_frame_count,
  },
  [XC2V250] = {
    .chip = XC2V250,
    .idcode = 0x01018093U,
    .framelen = 66,
    .col_count = {
      [V2C_IOB] = 2,
      [V2C_IOI] = 2,
      [V2C_CLB] = 16,
      [V2C_BRAM] = 4,
      [V2C_BRAM_INT] = 4,
      [V2C_GCLK] = 1,
    },
    .frame_count = v2_frame_count,
  },
  [XC2V500] = {
    .chip = XC2V500,
    .idcode = 0x01020093U,
    .framelen = 86,
    .col_count = {
      [V2C_IOB] = 2,
      [V2C_IOI] = 2,
      [V2C_CLB] = 24,
      [V2C_BRAM] = 4,
      [V2C_BRAM_INT] = 4,
      [V2C_GCLK] = 1,
    },
    .frame_count = v2_frame_count,
  },
  [XC2V1000] = {
    .chip = XC2V1000,
    .idcode = 0x01028093U,
    .framelen = 106,
    .col_count = {
      [V2C_IOB] = 2,
      [V2C_IOI] = 2,
      [V2C_CLB] = 32,
      [V2C_BRAM] = 4,
      [V2C_BRAM_INT] = 4,
      [V2C_GCLK] = 1,
    },
    .frame_count = v2_frame_count,
  },
  [XC2V1500] = {
    .chip = XC2V1500,
    .idcode = 0x01030093U,
    .framelen = 126,
    .col_count = {
      [V2C_IOB] = 2,
      [V2C_IOI] = 2,
      [V2C_CLB] = 40,
      [V2C_BRAM] = 4,
      [V2C_BRAM_INT] = 4,
      [V2C_GCLK] = 1,
    },
    .frame_count = v2_frame_count,
  },
  [XC2V2000] = {
    .chip = XC2V2000,
    .idcode = 0x01038093U,
    .framelen = 146,
    .col_count = {
      [V2C_IOB] = 2,
      [V2C_IOI] = 2,
      [V2C_CLB] = 48,
      [V2C_BRAM] = 4,
      [V2C_BRAM_INT] = 4,
      [V2C_GCLK] = 1,
    },
    .frame_count = v2_frame_count,
  },
  [XC2V3000] = {
    .chip = XC2V3000,
    .idcode = 0x01040093U,
    .framelen = 166,
    .col_count = {
      [V2C_IOB] = 2,
      [V2C_IOI] = 2,
      [V2C_CLB] = 56,
      [V2C_BRAM] = 6,
      [V2C_BRAM_INT] = 6,
      [V2C_GCLK] = 1,
    },
    .frame_count = v2_frame_count,
  },
  [XC2V4000] = {
    .chip = XC2V4000,
    .idcode = 0x01050093U,
    .framelen = 206,
    .col_count = {
      [V2C_IOB] = 2,
      [V2C_IOI] = 2,
      [V2C_CLB] = 72,
      [V2C_BRAM] = 6,
      [V2C_BRAM_INT] = 6,
      [V2C_GCLK] = 1,
    },
    .frame_count = v2_frame_count,
  },
  [XC2V6000] = {
    .chip = XC2V6000,
    .idcode = 0x01060093U,
    .framelen = 246,
    .col_count = {
      [V2C_IOB] = 2,
      [V2C_IOI] = 2,
      [V2C_CLB] = 88,
      [V2C_BRAM] = 6,
      [V2C_BRAM_INT] = 6,
      [V2C_GCLK] = 1,
    },
    .frame_count = v2_frame_count,
  },
  [XC2V8000] = {
    .chip = XC2V8000,
    .idcode = 0x01070093U,
    .framelen = 286,
    .col_count = {
      [V2C_IOB] = 2,
      [V2C_IOI] = 2,
      [V2C_CLB] = 104,
      [V2C_BRAM] = 6,
      [V2C_BRAM_INT] = 6,
      [V2C_GCLK] = 1,
    },
    .frame_count = v2_frame_count,
  },
};

#define CHIPS__NUM XC2__NUM

#elif defined(SPARTAN3)

/* xapp452 */
static const unsigned
s3_frame_count[V2C__NB_CFG] = {
  [V2C_IOB] = 2,
  [V2C_IOI] = 19,
  [V2C_CLB] = 19,
  [V2C_BRAM] = 76,
  [V2C_BRAM_INT] = 19,
  [V2C_GCLK] = 3,
};

/* Spartan-3 */
static const
chip_struct_t bitdescr[XC3__NUM] = {
  [XC3S50] = {
    .chip = XC3S50,
    .idcode = 0x0140D093U,
    .framelen = 37,
    .col_count = {
      [V2C_IOB] = 2,
      [V2C_IOI] = 2,
      [V2C_CLB] = 12,
      [V2C_BRAM] = 1,
      [V2C_BRAM_INT] = 1,
      [V2C_GCLK] = 1,
    },
    .frame_count = s3_frame_count,
  },
  [XC3S200] = {
    .chip = XC3S200,
    .idcode = 0x01414093U,
    .framelen = 53,
    .col_count = {
      [V2C_IOB] = 2,
      [V2C_IOI] = 2,
      [V2C_CLB] = 20,
      [V2C_BRAM] = 2,
      [V2C_BRAM_INT] = 2,
      [V2C_GCLK] = 1,
    },
    .frame_count = s3_frame_count,
  },
  [XC3S400] = {
    .chip = XC3S400,
    .idcode = 0x0141C093U,
    .framelen = 69,
    .col_count = {
      [V2C_IOB] = 2,
      [V2C_IOI] = 2,
      [V2C_CLB] = 28,
      [V2C_BRAM] = 2,
      [V2C_BRAM_INT] = 2,
      [V2C_GCLK] = 1,
    },
    .frame_count = s3_frame_count,
  },
  [XC3S1000] = {
    .chip = XC3S1000,
    .idcode = 0x11428093U,
    .framelen = 101,
    .col_count = {
      [V2C_IOB] = 2,
      [V2C_IOI] = 2,
      [V2C_CLB] = 40,
      [V2C_BRAM] = 2,
      [V2C_BRAM_INT] = 2,
      [V2C_GCLK] = 1,
    },
    .frame_count = s3_frame_count,
  },
  [XC3S1500] = {
    .chip = XC3S1500,
    .idcode = 0x01434093,
    .framelen = 133,
    .col_count = {
      [V2C_IOB] = 2,
      [V2C_IOI] = 2,
      [V2C_CLB] = 52,
      [V2C_BRAM] = 2,
      [V2C_BRAM_INT] = 2,
      [V2C_GCLK] = 1,
    },
    .frame_count = s3_frame_count,
  },
  [XC3S2000] = {
    .chip = XC3S2000,
    .idcode = 0x01440093,
    .framelen = 165,
    .col_count = {
      [V2C_IOB] = 2,
      [V2C_IOI] = 2,
      [V2C_CLB] = 64,
      [V2C_BRAM] = 2,
      [V2C_BRAM_INT] = 2,
      [V2C_GCLK] = 1,
    },
    .frame_count = s3_frame_count,
  },
  [XC3S4000] = {
    .chip = XC3S4000,
    .idcode = 0x01448093U,
    .framelen = 197,
    .col_count = {
      [V2C_IOB] = 2,
      [V2C_IOI] = 2,
      [V2C_CLB] = 72,
      [V2C_BRAM] = 4,
      [V2C_BRAM_INT] = 4,
      [V2C_GCLK] = 1,
    },
    .frame_count = s3_frame_count,
  },
  [XC3S5000] = {
    .chip = XC3S5000,
    .idcode = 0x01450093U,
    .framelen = 213,
    .col_count = {
      [V2C_IOB] = 2,
      [V2C_IOI] = 2,
      [V2C_CLB] = 80,
      [V2C_BRAM] = 4,
      [V2C_BRAM_INT] = 4,
      [V2C_GCLK] = 1,
    },
    .frame_count = s3_frame_count,
  },
};

#define CHIPS__NUM XC3__NUM

#endif

typedef enum parser_state {
  STATE_IDLE = 0,
  STATE_UNSYNCHED,
  STATE_WAITING_CTRL,
  STATE_WAITING_DATA,
} parser_state_t;

#if defined(VIRTEX2)

typedef enum _registers_index {
  /* from xilinx ug002.pdf */
  CRC = 0, FAR, FDRI, FDRO,
  CMD, CTL, MASK,
  STAT, LOUT, COR,
  MFWR, FLR, KEY,
  CBC, IDCODE,
  __NUM_REGISTERS,
} register_index_t;

typedef enum _cmd_code {
  WCFG = 1,
  C_MFWR,
  LFRM, RCFG, START, RCAP, RCRC,
  AGHIGH, SWITCH, GRESTORE,
  SHUTDOWN, GCAPTURE,
  DESYNCH,
  __NUM_CMD_CODE,
} cmd_code_t;

#if DEBIT_DEBUG > 0
static const
char *reg_names[__NUM_REGISTERS] = {
  [CRC] = "CRC",
  [FAR] = "FAR",
  [FDRI] = "FDRI",
  [FDRO] = "FDRO",
  [CMD] = "CMD",
  [CTL] = "CTL",
  [MASK] = "MASK",
  [STAT] = "STAT",
  [LOUT] = "LOUT",
  [COR] = "COR",
  [MFWR] = "MFWR",
  [FLR] = "FLR",
  [KEY] = "KEY",
  [CBC] = "CBC",
  [IDCODE] = "IDCODE",
};

static const
char *cmd_names[__NUM_CMD_CODE] = {
  [WCFG] = "WCFG",
  [C_MFWR] = "MFWR",
  [LFRM] = "LFRM",
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

#define chip_id_t v2_id_t

#elif defined(SPARTAN3)
/* Spartan-3, from xapp352 */
typedef enum _registers_index {
  /* from xilinx ug002.pdf */
  CRC = 0, FAR, FDRI, FDRO,
  CMD, CTL, MASK,
  STAT, LOUT, COR,
  MFWR, FLR, RESERVED1,
  RESERVED2, IDCODE,
  SNOWPLOW,
  __NUM_REGISTERS,
} register_index_t;

/* This is the same as virtex-2 */
typedef enum _cmd_code {
  C_NULL = 0, WCFG,
  C_MFWR,
  DGHIGH_LFRM, RCFG, START, RCAP, RCRC,
  AGHIGH, SWITCH, GRESTORE,
  SHUTDOWN, GCAPTURE,
  DESYNCH,
  __NUM_CMD_CODE,
} cmd_code_t;

#if DEBIT_DEBUG > 0
static const
char *reg_names[__NUM_REGISTERS] = {
  [CRC] = "CRC",
  [FAR] = "FAR",
  [FDRI] = "FDRI",
  [FDRO] = "FDRO",
  [CMD] = "CMD",
  [CTL] = "CTL",
  [MASK] = "MASK",
  [STAT] = "STAT",
  [LOUT] = "LOUT",
  [COR] = "COR",
  [MFWR] = "MFWR",
  [FLR] = "FLR",
  [RESERVED1] = "RESERVED1",
  [RESERVED2] = "RESERVED2",
  [IDCODE] = "IDCODE",
};

/* This is the same as virtex2 */
static const
char *cmd_names[__NUM_CMD_CODE] = {
  [C_NULL] = "NULL",
  [WCFG] = "WCFG",
  [C_MFWR] = "MFWR",
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

#endif /* DEBIT_DEBUG > 0 */

#define chip_id_t s3_id_t

#endif /* SPARTAN3 */

/* This structure contains the internal structure of the parser */
typedef struct _bitstream_parser {
  /* state of the parser */
  parser_state_t state;
  /* state of the register engine */
  register_index_t active_register;
  gint write__read;
  gint active_length;

  /* Actual state of the various registers */
  xil_register_t registers[__NUM_REGISTERS];

  /* detailed view of some registers */
  chip_id_t type;

  /* Specific FDRI quirks */
  const void *last_frame;

  /* Bitstream proper */
  bytearray_t ba;
} bitstream_parser_t;

/*
 * Returns the length, in words, of a frame, according to the flr value
 */

static inline guint
frame_length(const bitstream_parser_t *parser) {
  return parser->registers[FLR].value + 1;
}

/***
 * Raw register IO
 ***/

static inline guint32
register_read(const bitstream_parser_t *parser,
	      const register_index_t reg) {
  return parser->registers[reg].value;
}

static inline void
register_write(bitstream_parser_t *parser,
	       const register_index_t reg,
	       const guint32 val) {
  parser->registers[reg].value = val;
}

/***
 * CRC
 ***/

static inline void
update_crc(bitstream_parser_t *parser,
	   const register_index_t reg,
	   const guint32 val) {
  xil_register_t *crcreg = &parser->registers[CRC];
  guint32 bcc = crcreg->value;

  switch (reg) {
  case LOUT:
    return;
  default:
    break;
  }

  /* first go through the value bits */
  bcc = crc_ibm_byte(bcc, val);
  bcc = crc_ibm_byte(bcc, val >> 8);
  bcc = crc_ibm_byte(bcc, val >> 16);
  bcc = crc_ibm_byte(bcc, val >> 24);

  /* the 5 bits of register address */
  bcc = crc_ibm_addr5(bcc, reg);

  /* write the CRC to the CRC register */
  crcreg->value = bcc;

  /* writes to the CRC should yield a zero value.
     In case of strict checks, this should abort the parsing. */
  if (reg == CRC) {
    debit_log(L_BITSTREAM,"write to CRC register yielded %04x", bcc);
  }
}

/***
 * FAR
 ***/

static const
char *type_names[V2C__NB_CFG] = {
  [V2C_IOB] = "IOB",
  [V2C_IOI] = "IOI",
  [V2C_CLB] = "CLB",
  [V2C_BRAM] = "BRAM",
  [V2C_BRAM_INT] = "BRAM_INT",
  [V2C_GCLK] = "GCLK",
};

void
typed_frame_name(char *buf, unsigned buf_len,
		 const unsigned type,
		 const unsigned index,
		 const unsigned frameid) {
  snprintf(buf, buf_len, "frame_%s_%02x_%02x",
	   type_names[type], index, frameid);
}

int
snprintf_far(char *buf, const size_t buf_len,
	     const uint32_t hwfar) {
  return snprintf(buf, buf_len,
		  "%i_%i_%i_%i",
		  ba_of_far(hwfar),
		  mja_of_far(hwfar),
		  mna_of_far(hwfar),
		  bn_of_far(hwfar));
}

static inline void
print_far(sw_far_t *far) {
  debit_log(L_BITSTREAM, "FAR is [ba %i, mja %i, mna %i, bn %i]",
	    far->ba, far->mja, far->mna, far->bn);
  (void) far;
}

typedef enum _ba_col_type {
  BA_TYPE_CLB = 0,
  BA_TYPE_BRAM,
  BA_TYPE_BRAM_INT,
  BA_TYPE_END,
} ba_col_type_t;

static inline int
_last_frame(const chip_id_t chiptype,
	    const sw_far_t *addr) {
  /* last frame is seen how ?
     HACK HACK ugly */
  const unsigned *col_count = bitdescr[chiptype].col_count;
  const int cond = (addr->ba == BA_TYPE_BRAM_INT  &&
		    addr->mja == col_count[V2C_BRAM_INT]);
  return cond;
}

static inline int
_type_of_far(const chip_id_t chiptype,
	     const sw_far_t *addr) {
  int ba = addr->ba;

  /* See ug002, page 322, 340 */
  switch(ba) {
  case BA_TYPE_CLB: {
    const unsigned *col_count = bitdescr[chiptype].col_count;
    int nclb = col_count[V2C_CLB];
    int mja = addr->mja;

    if (mja == 0)
      return V2C_GCLK;

    if (mja == 1 || mja == nclb+4)
      return V2C_IOB;

    if (mja == 2 || mja == nclb+3)
      return V2C_IOI;

    return V2C_CLB;
  }
  case BA_TYPE_BRAM:
    return V2C_BRAM;
  case BA_TYPE_BRAM_INT:
    return V2C_BRAM_INT;
  default:
    debit_log(L_BITSTREAM,"Unrecognized ba type %i",ba);
    return -1;
  }
}

static inline int
_col_of_far(const chip_id_t chiptype,
	    const sw_far_t *addr) {
  int nclb = bitdescr[chiptype].col_count[V2C_CLB];

  int type = _type_of_far(chiptype, addr);
  int mja = addr->mja;

  switch (type) {
  case V2C_IOB:
    if (mja == 1)
      return 0;
    if (mja == nclb+4)
      return 1;
    g_assert_not_reached();

  case V2C_IOI:
    if (mja == 2)
      return 0;
    if (mja == nclb+3)
      return 1;
    g_assert_not_reached();

  case V2C_CLB:
    return mja - 3;

  case V2C_BRAM:
  case V2C_BRAM_INT:
    return mja;

  case V2C_GCLK:
    return 0;

  default:
    g_assert_not_reached();
    return -1;
  }
}

static inline void
_far_increment_mja(const chip_id_t chiptype,
		   sw_far_t *addr, const int type) {
  const unsigned *col_count = bitdescr[chiptype].col_count;
  guint mja;

  addr->mja += 1;
  mja = addr->mja;

  if ((type == V2C_IOB  && mja == col_count[V2C_CLB] + 5) ||
      (type == V2C_BRAM && mja == col_count[V2C_BRAM])) {
    addr->mja = 0;
    addr->ba += 1;
  }
}

static inline void
_far_increment_mna(const chip_id_t chiptype,
		   sw_far_t *addr) {
  const unsigned *frame_count = bitdescr[chiptype].frame_count;
  int type;

  addr->mna += 1;
  type = _type_of_far(chiptype, addr);

  if (addr->mna == frame_count[type]) {
    addr->mna = 0;
    _far_increment_mja(chiptype, addr, type);
  }
}

static inline void
far_increment_mna(bitstream_parser_t *bitstream) {
  sw_far_t far;
  fill_swfar(&far, register_read(bitstream, FAR));
  _far_increment_mna(bitstream->type, &far);
  print_far(&far);
  register_write(bitstream, FAR, get_hwfar(&far));
}

static inline void
_far_increment_bn(const bitstream_parser_t *bitstream,
		  sw_far_t *addr) {
  addr->bn += 1;

  if (addr->bn == frame_length(bitstream)) {
    addr->bn = 0;
    _far_increment_mna(bitstream->type, addr);
  }
}

static inline void
far_increment_bn(bitstream_parser_t *bitstream)
{
  sw_far_t far;
  fill_swfar(&far, register_read(bitstream, FAR));
  _far_increment_bn(bitstream, &far);
  register_write(bitstream, FAR, get_hwfar(&far));
}

/* Other special registers */

typedef struct _bitfield_t {
  /* length is implicit as the difference of lenghts */
  unsigned char off;
} bitfield_t;

typedef enum _ctl_fields {
  GTS_USR_B = 0,
  RSV0, RSV1,
  PERSIST, SBITS,
  RSV2, CTL_FIELD_LAST,
} ctl_fields_t;

#define STDNAME(x) [x] = #x

static const char * ctl_fields_name[CTL_FIELD_LAST] = {
  STDNAME(GTS_USR_B),
  STDNAME(RSV0), STDNAME(RSV1),
  STDNAME(PERSIST), STDNAME(SBITS),
  STDNAME(RSV2),
};

static const bitfield_t ctl_bitfields[CTL_FIELD_LAST+1] = {
  [GTS_USR_B] = { .off = 0 },
  [RSV0] = { .off = 1 },
  [RSV1] = { .off = 2 },
  [PERSIST] = { .off = 3 },
  [SBITS] = { .off = 4 },
  [RSV2] = { .off = 6 },
  [CTL_FIELD_LAST] = { .off = 7 },
};

/* MASK is identical */

typedef enum _cor_fields {
  GWE_CYCLE = 0,
  GTS_CYCLE, LOCK_CYCLE,
  MATCH_CYCLE, DONE_CYCLE,
  SSCLKSRC, OSCFSEL,
  SINGLE, DRIVE_DONE, DONE_PIPE,
  SHUT_RST_DCM, RSV, CRC_BYPASS,
  COR_FIELD_LAST,
} ctl_bits_t;

static const char * cor_fields_name[COR_FIELD_LAST] = {
  STDNAME(GWE_CYCLE),
  STDNAME(GTS_CYCLE), STDNAME(LOCK_CYCLE),
  STDNAME(MATCH_CYCLE), STDNAME(DONE_CYCLE),
  STDNAME(SSCLKSRC), STDNAME(OSCFSEL),
  STDNAME(SINGLE), STDNAME(DRIVE_DONE), STDNAME(DONE_PIPE),
  STDNAME(SHUT_RST_DCM), STDNAME(RSV), STDNAME(CRC_BYPASS),
};

static const bitfield_t cor_bitfields[COR_FIELD_LAST+1] = {
  [GWE_CYCLE] = { .off = 0 },
  [GTS_CYCLE] = { .off = 3 },
  [LOCK_CYCLE] = { .off = 6 },
  [MATCH_CYCLE] = { .off = 9 },
  [DONE_CYCLE] = { .off = 12 },
  [SSCLKSRC] = { .off = 15 },
  [OSCFSEL] = { .off = 17 },
  [SINGLE] = { .off = 23 },
  [DRIVE_DONE] = { .off = 24 },
  [DONE_PIPE] = { .off = 25 },
  [SHUT_RST_DCM] = { .off = 26 },
  [RSV] = { .off = 27 },
  [CRC_BYPASS] = { .off = 29 },
  [COR_FIELD_LAST] = { .off = 30 },
};

#define MSK(x) ((1<<x)-1)
static void
decode_reg(const uint32_t rval,
	   const unsigned nfields,
	   const bitfield_t bitfields[],
	   const char *names[]) {
  unsigned i;
  for(i = 0; i < nfields; i++) {
    unsigned offset = bitfields[i].off;
    unsigned flen = bitfields[i+1].off - offset;
    unsigned fval = (rval >> offset) & MSK(flen);
    if (fval) {
      debit_log(L_BITSTREAM, "%s=%i", names[i], fval);
      (void) names;
    }
  }
}

static inline uint32_t
setfiel(const uint32_t val,
	const unsigned field,
	const bitfield_t bitfields[]) {
  unsigned offset = bitfields[field].off;
  unsigned flen = bitfields[field+1].off - offset;
  assert((val & MSK(flen)) == val);
  return val << offset;
}

static inline void
default_register_write(bitstream_parser_t *parser,
		       const register_index_t reg,
		       const gsize length) {
  bytearray_t *ba = &parser->ba;
  xil_register_t *regp = &parser->registers[reg];
  unsigned i;

  debit_log(L_BITSTREAM,"Writing %zd words to register %s", length, reg_names[reg]);

  for (i = 0; i < length; i++) {
    guint32 val = bytearray_get_uint32(ba);
    update_crc(parser, reg, val);

    switch (reg) {
    case CRC:
      /* CRC write does not really write the crc register, only updates
	 it as a side-effect. On v2 this does not mean anything, as the
         CRC check is triggered by an auto-CRC word */
      break;
    case LOUT: {
      gchar far_name[32];
      snprintf_far(far_name, sizeof(far_name), val);
      g_print("LOUT: %08x\n", val);
      g_print("LOUT as FAR is [%i], %s\n", val, far_name);
      /* Fall through */
    }
    default:
      regp->value = val;
    }
  }
  parser->active_length -= length;

}

static inline
void record_frame(bitstream_parsed_t *parsed,
		  bitstream_parser_t *bitstream,
		  const guint32 myfar) {
  sw_far_t far;
  guint type, index, frame;
  const char *dataframe = bitstream->last_frame;
  const char **frame_loc;

  fill_swfar(&far, myfar);

  type = _type_of_far(bitstream->type, &far);
  index = _col_of_far(bitstream->type, &far);
  frame = far.mna;

  debit_log(L_BITSTREAM,"flushing frame [type:%i,index:%02i,frame:%2X]",
	    type, index, frame);

  frame_loc = get_frame_loc(parsed, type, index, frame);
  if (*frame_loc != NULL)
	  g_warning("Overwriting already present frame");
  *frame_loc = dataframe;
}

static inline gsize
total_frame_count(bitstream_parsed_t *parsed) {
  const chip_struct_t *chip_struct = parsed->chip_struct;
  const unsigned *col_count = chip_struct->col_count;
  const unsigned *frame_count = chip_struct->frame_count;
  gsize total_count = 0;
  gsize type;

  for (type = 0; type < V2C__NB_CFG; type++)
    total_count += frame_count[type] * col_count[type];

  return total_count;
}

static void
alloc_indexer(bitstream_parsed_t *parsed) {
  const chip_struct_t *chip_struct = parsed->chip_struct;
  const unsigned *col_count = chip_struct->col_count;
  const unsigned *frame_count = chip_struct->frame_count;

  gsize total_size = 0;
  gsize type_offset = 0;
  gsize type;
  const gchar ***type_lut, **frame_array;

  /* We need room for the type lookup */
  total_size += V2C__NB_CFG * sizeof(gchar **);

  /* NB: We don't memoize the column lookup -- we prefer a multiplication
     for this  */
  total_size += total_frame_count(parsed) * sizeof(gchar *);

  /* We allocate only one big array with the type_lut at the beginning
     and the frame_array at the end */
  type_lut = g_new0(const gchar **, total_size);
  frame_array = (const gchar **) &type_lut[V2C__NB_CFG];

  /* fill in the control data */
  for (type = 0; type < V2C__NB_CFG; type++) {
    type_lut[type] = &frame_array[type_offset];
    type_offset += col_count[type] * frame_count[type];
  }

  parsed->frames = type_lut;

}

static inline void
free_indexer(bitstream_parsed_t *parsed) {
  gchar ***frames = (gchar ***)parsed->frames;
  if (frames)
    g_free(frames);
  parsed->frames = NULL;
}

/* XXX these functions burden the read-only case. To be put elsewhere
   at some point */
static void
fill_indexer(bitstream_parsed_t *parsed) {
  gchar **frame_array = (gchar **) &parsed->frames[V2C__NB_CFG];
  const chip_struct_t *chip_struct = parsed->chip_struct;
  const unsigned total_frames = total_frame_count(parsed);
  const unsigned framelen = chip_struct->framelen;
  uint32_t *current_frame;
  unsigned i;

  /* Alloc *all* frames. A bit tedious... */
  current_frame = g_new0(uint32_t, total_frames * framelen);

  for(i = 0; i < total_frames; i++) {
    frame_array[i] = (char *)current_frame;
    current_frame += framelen;
  }
}

static void
empty_indexer(bitstream_parsed_t *parsed) {
  gchar **frame_array = (gchar **) &parsed->frames[V2C__NB_CFG];
  /* Free *all* frames at once. Easy... */
  g_free (frame_array[0]);
}


int
alloc_wbitstream(bitstream_parsed_t *parsed) {
  /* Lookup the name */
  const header_option_p *devopt = get_option(&parsed->header, DEVICE_TYPE);
  const char *name = devopt->data;
  int i;

  for (i = CHIPS__NUM - 1; i >= 0; i--) {
    const char *chipname = chipfiles[i];
    if (!strncmp(name,chipname,strlen(chipname))) {
      parsed->chip_struct = &bitdescr[i];
      alloc_indexer(parsed);
      fill_indexer(parsed);
      return 0;
    }
  }
  return -1;
}

void
free_wbitstream(bitstream_parsed_t *parsed) {
  empty_indexer(parsed);
  free_indexer(parsed);
}

/* XXX End-of-burden */

void
iterate_over_frames(const bitstream_parsed_t *parsed,
		    frame_iterator_t iter, void *itdat) {
  const chip_struct_t *chip_struct = parsed->chip_struct;
  const unsigned *col_counts = chip_struct->col_count;
  const unsigned *frame_counts = chip_struct->frame_count;
  guint type, index, frame;

  /* Iterate over the whole thing */
  for (type = 0; type < V2C__NB_CFG; type++) {
    const guint col_count = col_counts[type];

    for (index = 0; index < col_count; index++) {
      const guint frame_count = frame_counts[type];

      for (frame = 0; frame < frame_count; frame++) {
	const gchar *data = get_frame(parsed, type, index, frame);
	iter(data, type, index, frame, itdat);
      }
    }
  }
}

/* Get chip ID directly */
static chip_id_t chipid(const bitstream_parsed_t *parsed) {
  const chip_struct_t *chip = parsed->chip_struct;
  return chip - bitdescr;
}

/* Iterate over frames in FAR-ordered mode. This is a bit complex... */
void
iterate_over_frames_far(const bitstream_parsed_t *parsed,
			frame_iterator_t iter, void *itdat) {
  chip_id_t chip_id = chipid(parsed);
  sw_far_t far;
  fill_swfar(&far, 0);

  /* Iterate over the whole thing very dumbly */
  while (!_last_frame(chip_id, &far)) {
    const int type = _type_of_far(chip_id, &far);
    const int index = _col_of_far(chip_id, &far);
    const int frame = far.mna;
    const gchar *data = get_frame(parsed, type, index, frame);

    iter(data, type, index, frame, itdat);
    _far_increment_mna(chip_id, &far);
  }
}

void iterate_over_unk_frames(const bitstream_parsed_t *parsed,
			     frame_unk_iterator_t iter, void *itdat) {
  (void) parsed;
  (void) iter;
  (void) itdat;
  return;
}

static gint
handle_fdri_write(bitstream_parsed_t *parsed,
		  bitstream_parser_t *parser,
		  const unsigned length) {
  bytearray_t *ba = &parser->ba;
  const gchar *frame = bytearray_get_ptr(ba);
  const guint frame_len = frame_length(parser);
  guint i, nrframes;
  guint32 last_far = 0;

  /* Frame length writes must be a multiple of the flr length */
  if (length % frame_len) {
    debit_log(L_BITSTREAM,"%i bytes in FDRI write, "
	      "which is inconsistent with the FLR value %i",
	      length, frame_len);
    return -1;
  }

  nrframes = length / frame_len;

  /* We handle here a complete series of writes, so that we have
     the ability to see the start and end frames */

  for (i = 0; i < nrframes; i++) {

    /* The first write of a FDRI write in WCFG mode does not flush the
       previous writes. As I don't know what other modes may be on, be
       conservative wrt to mode setting */
    if (i != 0)
      /* flush the previous frame into the frame array with the previous FAR
	 address */
      record_frame(parsed, parser, last_far);

    last_far = register_read(parser, FAR);
    parser->last_frame = frame;

    /* evolution of the state machine */
    far_increment_mna(parser);
    frame += frame_len * sizeof(guint32);
  }

  debit_log(L_BITSTREAM,"%i frames written to fdri", i);

  /* Tell the others that we need an autoCRC word */
  return length+1;
}

static gint
handle_cmd_write(bitstream_parsed_t *parsed,
		 bitstream_parser_t *parser) {
  cmd_code_t cmd = register_read(parser, CMD);

  switch(cmd) {
  case C_MFWR:
    debit_log(L_BITSTREAM,"Executing multi-frame write");
    record_frame(parsed, parser, register_read(parser, FAR));
    break;
  case RCRC:
    debit_log(L_BITSTREAM,"Resetting CRC");
    register_write(parser, CRC, 0);
    break;
  default:
    debit_log(L_BITSTREAM,"Command %s",cmd_names[cmd]);
    break;
  }

  return 0;

}

static inline gint
flr_check(const bitstream_parser_t *parser, chip_id_t chip) {
  guint32 flr = register_read(parser, FLR);
  guint32 chipflr = bitdescr[chip].framelen;

  if (chipflr != flr + 1) {
    g_warning("written FLR value %i is inconsistent with expected value %i",
	      flr, chipflr-1);
    return -1;
  }

  return 0;
}

static gint
idcode_write(bitstream_parsed_t *parsed,
	     bitstream_parser_t *parser) {
  guint32 idcode = register_read(parser, IDCODE);
  int i;

  for (i = 0; i < CHIPS__NUM; i++)
    if (bitdescr[i].idcode == idcode) {
      parser->type = i;
      parsed->chip_struct = &bitdescr[i];
      /* Allocate control structures */
      alloc_indexer(parsed);
      return flr_check(parser, i);
    }

  g_warning("IDCODE %08x not recognized, aborting", idcode);
  return -1;
}

static int
synchronize_bitstream(bitstream_parser_t *parser) {
  bytearray_t *ba = &parser->ba;
  guint32 synch;

  /* XXX guint32 data access must be aligned -- make sure it is */
  /* advance the bitstream until the sync word is found */
  do {
    synch = bytearray_get_uint32(ba);
  } while (synch != SYNCHRO);

  parser->state = STATE_WAITING_CTRL;
  return 0;
}

static gint
read_next_token(bitstream_parsed_t *parsed, bitstream_parser_t *parser);

/* function to allow parsing */
static int
_parse_bitstream_data(bitstream_parsed_t *dest,
		      bitstream_parser_t *parser) {
  gint advance;
  int err;
  /* First parse the header of the bitstream */

  /* Then synchronize the bitstream data */
  parser->state = STATE_UNSYNCHED;

  err = synchronize_bitstream(parser);
  if (err) {
    debit_log(L_BITSTREAM,"Could not synchronize bitstream");
    return err;
  }

  /* Then launch the computation */
  do {
    advance = read_next_token(dest, parser);
  } while(advance > 0);

  if (advance < 0) {
    debit_log(L_BITSTREAM,"Error parsing bitstream: %i", advance);
  }

  return advance;
}

static inline void
print_parser_state(const bitstream_parser_t *parser) {
  gint state = parser->state;
  switch(state) {
  case STATE_WAITING_CTRL:
    debit_log(L_BITSTREAM,"Waiting CTRL");
    break;
  case STATE_WAITING_DATA:
    debit_log(L_BITSTREAM,"Waiting DATA, %i words remaining for register %s",
	      parser->active_length, reg_names[parser->active_register]);
    break;
  default:
    debit_log(L_BITSTREAM,"Unknown parser state %i",state);
  }
}

/**
 *
 * @returns the number of words read
 */

static int
read_next_token(bitstream_parsed_t *parsed,
		bitstream_parser_t *parser) {
  gint state = parser->state;
  bytearray_t *ba = &parser->ba;
  unsigned offset = 1;
  int err = 0;

  print_parser_state(parser);

  switch(state) {
  case STATE_WAITING_CTRL:
    {
      gsize avail = bytearray_available(ba);
      guint32 pkt;

      /* For now we don't error out in this state */
      if (avail == 0) {
	debit_log(L_BITSTREAM,"End-of-bitstream reached");
	return 0;
      }

      pkt = bytearray_get_uint32(ba);

      /* catch a noop */
      if (pkt == NOOP) {
	debit_log(L_BITSTREAM,"Got NOOP packet");
	return offset;
      }

      /* v1 or v2 packet */
      switch (type_of_pkt1(pkt)) {
      case TYPE_V1: {
	debit_log(L_BITSTREAM,"Got V1 packet");
	parser->active_register = rega_of_pkt1(pkt);
	parser->active_length = wordc_of_pkt1(pkt);
	parser->write__read = wr_of_pkt1(pkt);
	break;
      }
      case TYPE_V2: {
	debit_log(L_BITSTREAM,"Got V2 packet");
	parser->active_length = wordc_of_v2pkt(pkt);
	break;
      }
      case NOOP:
	break;

      default:
	debit_log(L_BITSTREAM,"Unrecognized packet %08x while in state %i", pkt, state);
	return -1;
      }

      /* If there is data to read, then switch state */
      if (parser->active_length > 0)
	parser->state = STATE_WAITING_DATA;

    }
    break;
  case STATE_WAITING_DATA:
    {
      register_index_t reg = parser->active_register;
      gsize avail = bytearray_available(ba),
	length = parser->active_length;

      offset = length;

      /* pre-processing */
      switch (reg) {
      case FDRI:
	offset = handle_fdri_write(parsed, parser, length);
	break;
      default:
	break;
      }

      if (offset > avail) {
	debit_log(L_BITSTREAM,"Register length of %zd words while only %zd words remain",
		  length, avail);
	return -1;
      }

      /* This function does the CRC update */
      default_register_write(parser, reg, length);

      /* post-processing */
      switch(reg) {
      case FDRI:
	{
	  /* Handle the AutoCRC word */
	  guint32 autocrc = bytearray_get_uint32(ba);
	  debit_log(L_BITSTREAM,"FDRI write with CMD register %s",
		    cmd_names[register_read(parser,CMD)]);
	  debit_log(L_BITSTREAM,"FDRI handling autoCRC");
	  update_crc(parser, CRC, autocrc);
	  break;
	}
      case FAR:
	debit_log(L_BITSTREAM,"FAR write reexecuting CMD register");
	/* Fall-through to CMD register action */
      case CMD:
	err = handle_cmd_write(parsed, parser);
	break;
      case IDCODE:
	/* get the index of the IDCODE & check FLR consistency */
	err = idcode_write(parsed, parser);
	break;
      case MASK:
      case CTL:
	decode_reg(register_read(parser, reg), CTL_FIELD_LAST,
		   ctl_bitfields, ctl_fields_name);
	break;
      case COR:
	decode_reg(register_read(parser, reg), COR_FIELD_LAST,
		   cor_bitfields, cor_fields_name);
	break;
      default:
	break;
      }

      if (err)
	return -1;

      /* < 0 happens with autocrc on FDRI writes */
      if (parser->active_length <= 0)
	parser->state = STATE_WAITING_CTRL;

    }
  }

  return offset;
}

#include "bitstream_high.h"
