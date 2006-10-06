/*
 * (C) Copyright 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
 * All rights reserved.
 *
 */

#include <glib.h>
#include <string.h>

#include "bitarray.h"
#include "design.h"
#include "bitheader.h"
#include "bitstream_parser.h"
#include "debitlog.h"

/* This is nothing but a key-value file */
static const
chip_struct_t bitdescr[XC2__NUM] = {
  [XC2V40] = {
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
    .frame_count = {
      [V2C_IOB] = 4,
      [V2C_IOI] = 22,
      [V2C_CLB] = 22,
      [V2C_BRAM] = 64,
      [V2C_BRAM_INT] = 22,
      [V2C_GCLK] = 4,
    },
  },
  [XC2V80] = {
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
    .frame_count = {
      [V2C_IOB] = 4,
      [V2C_IOI] = 22,
      [V2C_CLB] = 22,
      [V2C_BRAM] = 64,
      [V2C_BRAM_INT] = 22,
      [V2C_GCLK] = 4,
    },
  },
  [XC2V250] = {
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
    .frame_count = {
      [V2C_IOB] = 4,
      [V2C_IOI] = 22,
      [V2C_CLB] = 22,
      [V2C_BRAM] = 64,
      [V2C_BRAM_INT] = 22,
      [V2C_GCLK] = 4,
    },
  },
  [XC2V500] = {
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
    .frame_count = {
      [V2C_IOB] = 4,
      [V2C_IOI] = 22,
      [V2C_CLB] = 22,
      [V2C_BRAM] = 64,
      [V2C_BRAM_INT] = 22,
      [V2C_GCLK] = 4,
    },
  },
  [XC2V1000] = {
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
    .frame_count = {
      [V2C_IOB] = 4,
      [V2C_IOI] = 22,
      [V2C_CLB] = 22,
      [V2C_BRAM] = 64,
      [V2C_BRAM_INT] = 22,
      [V2C_GCLK] = 4,
    },
  },
  [XC2V1500] = {
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
    .frame_count = {
      [V2C_IOB] = 4,
      [V2C_IOI] = 22,
      [V2C_CLB] = 22,
      [V2C_BRAM] = 64,
      [V2C_BRAM_INT] = 22,
      [V2C_GCLK] = 4,
    },
  },
  [XC2V2000] = {
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
    .frame_count = {
      [V2C_IOB] = 4,
      [V2C_IOI] = 22,
      [V2C_CLB] = 22,
      [V2C_BRAM] = 64,
      [V2C_BRAM_INT] = 22,
      [V2C_GCLK] = 4,
    },
  },
  [XC2V3000] = {
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
    .frame_count = {
      [V2C_IOB] = 4,
      [V2C_IOI] = 22,
      [V2C_CLB] = 22,
      [V2C_BRAM] = 64,
      [V2C_BRAM_INT] = 22,
      [V2C_GCLK] = 4,
    },
  },
  [XC2V4000] = {
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
    .frame_count = {
      [V2C_IOB] = 4,
      [V2C_IOI] = 22,
      [V2C_CLB] = 22,
      [V2C_BRAM] = 64,
      [V2C_BRAM_INT] = 22,
      [V2C_GCLK] = 4,
    },
  },
  [XC2V6000] = {
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
    .frame_count = {
      [V2C_IOB] = 4,
      [V2C_IOI] = 22,
      [V2C_CLB] = 22,
      [V2C_BRAM] = 64,
      [V2C_BRAM_INT] = 22,
      [V2C_GCLK] = 4,
    },
  },
  [XC2V8000] = {
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
    .frame_count = {
      [V2C_IOB] = 4,
      [V2C_IOI] = 22,
      [V2C_CLB] = 22,
      [V2C_BRAM] = 64,
      [V2C_BRAM_INT] = 22,
      [V2C_GCLK] = 4,
    },
  },
};

typedef enum parser_state {
  STATE_IDLE = 0,
  STATE_UNSYNCHED,
  STATE_WAITING_CTRL,
  STATE_WAITING_DATA,
} parser_state_t;

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

typedef enum _cmd_pkt_ver {
  V1 = 1, V2 =2,
} cmd_pkt_ver_t;

typedef enum _special_words {
  SYNCHRO = 0xAA995566U,
  NOOP    = 0x20000000U,
} special_word_t;

typedef struct _xil_register {
  guint32 value;
} xil_register_t;

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
  id_t type;

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

/* Uses the lowest bit of the bit argument */

static inline guint16
shift_one_crc_bit(guint bit, guint16 bcc) {
  guint val = ((bcc >> 15) ^ bit) & 1;
  if (val != 0) {
    bcc <<= 1;
    bcc ^= 0x8005;
  } else {
    bcc <<= 1;
  }

  return bcc & 0xffff;
}

static inline void
update_crc(bitstream_parser_t *parser,
	   const register_index_t reg,
	   const guint32 val) {
  unsigned i;
  xil_register_t *crcreg = &parser->registers[CRC];
  guint32 bcc = crcreg->value;

  /* first go through the value bits */
  for (i=0; i < 32; i++)
    bcc = shift_one_crc_bit(val >> i, bcc);

  /* possibly 5 bits of address */
#define V2_REG_ADDR_BITS 5
  for (i=0; i < V2_REG_ADDR_BITS; i++)
    bcc = shift_one_crc_bit(reg >> i, bcc);

  /* write the CRC to the CRC register */
  crcreg->value = bcc;

  /* writes to the CRC should yield a zero value.
     In case of strict checks, this should abort the parsing. */
  if (reg == CRC)
    debit_log(L_BITSTREAM,"write to CRC register yielded %04x", bcc);
}

/***
 * FAR
 ***/

static inline void
print_far(sw_far_t *far) {
  debit_log(L_BITSTREAM, "FAR is [ba %i, mja %i, mna %i, bn %i]",
	    far->ba, far->mja, far->mna, far->bn);
}

typedef enum _ba_col_type {
  BA_TYPE_CLB = 0,
  BA_TYPE_BRAM,
  BA_TYPE_BRAM_INT,
} ba_col_type_t;

static inline int
_type_of_far(const bitstream_parser_t *bitstream,
	     const sw_far_t *addr) {
  int ba = addr->ba;

  /* See ug002, page 322, 340 */
  switch(ba) {
  case BA_TYPE_CLB: {
    id_t chiptype = bitstream->type;
    const int *col_count = bitdescr[chiptype].col_count;
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
_col_of_far(const bitstream_parser_t *bitstream,
	    const sw_far_t *addr) {
  id_t chiptype = bitstream->type;
  int nclb = bitdescr[chiptype].col_count[V2C_CLB];

  int type = _type_of_far(bitstream, addr);
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
_far_increment_mja(bitstream_parser_t *bitstream,
		   sw_far_t *addr, int type) {
  id_t chiptype = bitstream->type;
  const int *col_count = bitdescr[chiptype].col_count;
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
_far_increment_mna(bitstream_parser_t *bitstream,
		   sw_far_t *addr) {
  id_t chiptype = bitstream->type;
  const int *frame_count = bitdescr[chiptype].frame_count;
  int type;

  addr->mna += 1;
  type = _type_of_far(bitstream, addr);

  if (addr->mna == frame_count[type]) {
    addr->mna = 0;
    _far_increment_mja(bitstream, addr, type);
  }
}

static inline void
far_increment_mna(bitstream_parser_t *bitstream) {
  sw_far_t far;
  fill_swfar(&far, register_read(bitstream, FAR));
  _far_increment_mna(bitstream, &far);
  print_far(&far);
  register_write(bitstream, FAR, get_hwfar(&far));
}

static inline void
_far_increment_bn(bitstream_parser_t *bitstream,
		  sw_far_t *addr) {
  addr->bn += 1;

  if (addr->bn == frame_length(bitstream)) {
    addr->bn = 0;
    _far_increment_mna(bitstream, addr);
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
    /* XXX writes to the CRC registers may only be crc updates */
    regp->value = val;
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

  fill_swfar(&far, myfar);

  type = _type_of_far(bitstream, &far);
  index = _col_of_far(bitstream, &far);
  frame = far.mna;

  debit_log(L_BITSTREAM,"flushing frame [type:%i,index:%02i,frame:%2X]",
	    type, index, frame);
  *get_frame_loc(parsed, type, index, frame) = dataframe;
}

static void
alloc_indexer(bitstream_parsed_t *parsed) {
  const chip_struct_t *chip_struct = parsed->chip_struct;
  const int *col_count = chip_struct->col_count;
  const int *frame_count = chip_struct->frame_count;

  gsize total_size = 0;
  gsize type_offset = 0;
  gsize type;
  const gchar ***type_lut, **frame_array;

  /* We need room for the type lookup */
  total_size += V2C__NB_CFG * sizeof(gchar **);

  /* NB: We don't memoize the column lookup -- we prefer a multiplication
     for this  */
  for (type = 0; type < V2C__NB_CFG; type++)
    total_size += frame_count[type] * col_count[type] * sizeof(gchar *);

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
}

void
iterate_over_frames(const bitstream_parsed_t *parsed,
		    frame_iterator_t iter, void *itdat) {
  const chip_struct_t *chip_struct = parsed->chip_struct;
  const int *col_counts = chip_struct->col_count;
  const int *frame_counts = chip_struct->frame_count;
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
    break;
  }

  return 0;

}

static inline gint
flr_check(const bitstream_parser_t *parser, id_t chip) {
  guint32 flr = register_read(parser, FLR);
  guint32 chipflr = bitdescr[chip].framelen;

  if (chipflr != flr + 1) {
    g_warning("written FLR value %i is inconsistent with expected value %i",
	      flr, chipflr);
    return -1;
  }

  return 0;
}

static gint
idcode_write(bitstream_parsed_t *parsed,
	     bitstream_parser_t *parser) {
  guint32 idcode = register_read(parser, IDCODE);
  int i;

  for (i = 0; i < XC2__NUM; i++)
    if (bitdescr[i].idcode == idcode) {
      parser->type = i;
      parsed->chip = i;
      parsed->chip_struct = &bitdescr[i];
      /* Allocate control structures */
      alloc_indexer(parsed);
      return flr_check(parser, i);
    }

  g_warning("IDCODE %08x not recognized, aborting", idcode);
  return -1;
}

int synchronize_bitstream(bitstream_parser_t *parser) {
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

  if (advance < 0)
    debit_log(L_BITSTREAM,"Error parsing bitstream: %i", advance);

  return advance;
}

static int
_parse_bitfile(bitstream_parsed_t *dest,
	       const char *buf_in, const off_t len) {
  int offset;
  bitstream_parser_t parser;

  /* memset the parser */
  memset(&parser, 0, sizeof(parser));

  /* parse the header */
  offset = parse_header(buf_in, len);

  if (offset < 0) {
    debit_log(L_BITSTREAM,"header parsing error");
    return -1;
  }

  /* Do some allocations according to the type of the bitfile */
  bytearray_init(&parser.ba, len, offset, buf_in);

  return _parse_bitstream_data(dest, &parser);
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

static gint
read_next_token(bitstream_parsed_t *parsed,
		bitstream_parser_t *parser) {
  gint state = parser->state;
  bytearray_t *ba = &parser->ba;
  int offset = 1;
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
      case V1: {
	debit_log(L_BITSTREAM,"Got V1 packet");
	parser->active_register = rega_of_pkt1(pkt);
	parser->active_length = wordc_of_pkt1(pkt);
	parser->write__read = wr_of_pkt1(pkt);
	break;
      }
      case V2: {
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

/* High-level functions */

bitstream_parsed_t *
parse_bitstream(const gchar*filename) {
  bitstream_parsed_t *dest;

  GError *error = NULL;
  GMappedFile *file = NULL;
  int err = 0;

  dest = g_new0(bitstream_parsed_t, 1);

  file = g_mapped_file_new (filename, FALSE, &error);
  dest->file = file;

  if (error != NULL) {
    debit_log(L_BITSTREAM,"could not map file %s: %s",filename,error->message);
    g_error_free (error);
    goto out_free_dest;
  }

  err = _parse_bitfile(dest,
		       g_mapped_file_get_contents (file),
		       g_mapped_file_get_length (file));

  if (err < 0)
    goto out_free;

  /* XXX Leak file */
  return dest;

 out_free:
  g_mapped_file_free (file);
 out_free_dest:
  g_free (dest);
  return NULL;
}

void
free_bitstream(bitstream_parsed_t *bitstream) {
  free_indexer(bitstream);
  g_mapped_file_free(bitstream->file);
  g_free(bitstream);
  return;
}
