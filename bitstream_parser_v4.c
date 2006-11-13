/*
 * (C) Copyright 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
 * All rights reserved.
 * Virtex4 bitstream parser.
 */

#include <glib.h>
#include <string.h>

#include "bitarray.h"
#include "bitstream_packets.h"
#include "design_v4.h"
#include "bitheader.h"
#include "bitstream_parser.h"
#include "debitlog.h"

typedef enum _id_v4 {
  XC4VLX15 = 0,
  XC4VLX25, XC4VLX40,
  XC4VLX60, XC4VLX80,
  XC4VLX100, XC4VLX160,
  XC4VLX200, XC4VLX__NUM,
} id_v4vlx_t;

static v4_frame_count[V4C__NB_CFG] = {
  [V4C_IOB] = 30,
  [V4C_GCLK] = 3,
  [V4C_CLB] = 22,
  [V4C_DSP48] = 21,
  [V4C_BRAM] = 64,
  [V4C_BRAM_INT] = 20,
  /* the padding frames are seen as a compulsory column
     of two frames at the end of any row */
  [V4C_PAD] = 2,
};

static const
chip_struct_t bitdescr[XC4VLX__NUM] = {
  /* FLR is always 41 for virtex-4 */
  [XC4VLX15] = { .idcode = 0x01658093,
		 .framelen = 41,
		 .frame_count = v4_frame_count,
		 .col_count = {
		   [V4_TYPE_CLB] = 29,
		   [V4_TYPE_BRAM] = 3,
		   [V4_TYPE_BRAM_INT] = 3,
		 },
		 .row_count = 2, },
  [XC4VLX25] = { .idcode = 0x0167C093,
		 .framelen = 41,
		 .frame_count = v4_frame_count,
		 .col_count = {
		   [V4_TYPE_CLB] = 33,
		   [V4_TYPE_BRAM] = 3,
		   [V4_TYPE_BRAM_INT] = 3,
		 },
		 .row_count = 3, },
  [XC4VLX40] = { .idcode = 0x016A4093,
		 .framelen = 41,
		 .frame_count = v4_frame_count,
		 .col_count = {
		   [V4_TYPE_CLB] = 41,
		   [V4_TYPE_BRAM] = 3,
		   [V4_TYPE_BRAM_INT] = 3,
		 },
		 .row_count = 4, },
  [XC4VLX60] = { .idcode = 0x016B4093,
		 .framelen = 41,
		 .frame_count = v4_frame_count,
		 .col_count = {
		   [V4_TYPE_CLB] = 57,
		   [V4_TYPE_BRAM] = 4,
		   [V4_TYPE_BRAM_INT] = 4,
		 },
		 .row_count = 5, },
  [XC4VLX80] = { .idcode = 0x016D8093,
		 .framelen = 41,
		 .frame_count = v4_frame_count,
		 .col_count = {
		   [V4_TYPE_CLB] = 61,
		   [V4_TYPE_BRAM] = 5,
		   [V4_TYPE_BRAM_INT] = 5,
		 },
		 .row_count = 5, },

  [XC4VLX100] = { .idcode = 0x01700093,
		  .framelen = 41,
		  .frame_count = v4_frame_count,
		  .col_count = {
		    [V4_TYPE_CLB] = 69,
		    [V4_TYPE_BRAM] = 5,
		    [V4_TYPE_BRAM_INT] = 5,
		  },
		  .row_count = 6, },
  [XC4VLX160] = { .idcode = 0x01718093,
		  .framelen = 41,
		  .frame_count = v4_frame_count,
		  .col_count = {
		    [V4_TYPE_CLB] = 93,
		    [V4_TYPE_BRAM] = 6,
		    [V4_TYPE_BRAM_INT] = 6,
		  },
		  .row_count = 6, },
  [XC4VLX200] = { .idcode = 0x01734093,
		  .framelen = 41,
		  .frame_count = v4_frame_count,
		  .col_count = {
		    [V4_TYPE_CLB] = 121,
		    [V4_TYPE_BRAM] = 7,
		    [V4_TYPE_BRAM_INT] = 7,
		  },
		  .row_count = 6, },
};

typedef enum _ba_v4_col_type {
  BA_TYPE_CLB = 0,
  BA_TYPE_BRAM_INT,
  BA_TYPE_BRAM,
} ba_v4_col_type_t;

typedef enum parser_state {
  STATE_IDLE = 0,
  STATE_UNSYNCHED,
  STATE_WAITING_CTRL,
  STATE_WAITING_DATA,
} parser_state_t;

typedef enum _v4_registers_index {
  /* from xilinx ug071.pdf */
  CRC = 0, FAR, FDRI, FDRO,
  CMD, CTL, MASK,
  STAT, LOUT, COR,
  MFWR, CBC, IDCODE,
  AXSS,
  __V4_NUM_REGISTERS,
} register_index_t;

typedef enum _cmd_code {
  CMD_NULL = 0,
  WCFG, C_MFWR,
  LFRM, RCFG, START, RCAP, RCRC,
  AGHIGH, SWITCH, GRESTORE,
  SHUTDOWN, GCAPTURE,
  DESYNCH,
  __NUM_CMD_CODE,
} cmd_code_t;

#if DEBIT_DEBUG > 0
static const
char *reg_names[__V4_NUM_REGISTERS] = {
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
  [CBC] = "CBC",
  [IDCODE] = "IDCODE",
  [AXSS] = "AXSS",
};

static const
char *cmd_names[__NUM_CMD_CODE] = {
  [CMD_NULL] = "NULL",
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
  TYPE_V1 = 1, TYPE_V2 = 2,
} cmd_pkt_ver_t;

typedef enum _special_words {
  SYNCHRO = 0xAA995566U,
  NOOP    = 0x20000000U,
  NULLPKT = 0x00000000U,
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
  xil_register_t registers[__V4_NUM_REGISTERS];

  /* detailed view of some registers */
  id_t type;

  /* Specific FDRI quirks */
/*   gboolean fdri_direct_mode; */
  const void *last_frame;

  /* Bitstream proper */
  bytearray_t ba;
} bitstream_parser_t;

/*
 * Returns the length, in words, of a frame, according to the flr value
 */

static inline guint
frame_length(const bitstream_parser_t *parser) {
  return 41;
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

static inline guint32
shift_one_crc_bit(guint bit, guint32 bcc) {
  guint val = ((bcc >> 31) ^ bit) & 1;
  if (val != 0) {
    bcc <<= 1;
    bcc ^= 0x1edc6f41;
  } else {
    bcc <<= 1;
  }

  return bcc;
}

static inline void
update_crc(bitstream_parser_t *parser,
	   const register_index_t reg,
	   const guint32 val) {
  unsigned i;
  xil_register_t *crcreg = &parser->registers[CRC];
  guint32 bcc = crcreg->value;

  switch (reg) {
  case LOUT:
    return;
  default:
    break;
  }

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
 * FAR -- no far decoding thus far.
 ***/

/***
 * FAR handling
 */

static inline void
print_far(bitstream_parser_t *parser) {
  const guint32 far = register_read(parser, FAR);
  gchar far_name[32];
  snprintf_far_v4(far_name, sizeof(far_name), far);
  debit_log(L_BITSTREAM, "FAR is [%i], %s", far, far_name);
}

static inline void
_far_increment_type(sw_far_v4_t *addr) {
  addr->type++;
}

static inline void
_far_increment_tb(sw_far_v4_t *addr) {
  v4_tb_t tb = addr->tb;
  switch (tb) {
  case V4_TB_TOP:
    addr->tb = V4_TB_BOTTOM;
    break;
  case V4_TB_BOTTOM:
    addr->tb = V4_TB_TOP;
    /* increase type */
    _far_increment_type(addr);
    break;
  default:
    g_assert_not_reached();
  }
}

static inline void
_far_increment_row(bitstream_parser_t *bitstream,
		   sw_far_v4_t *addr) {
  const id_t chiptype = bitstream->type;
  const unsigned row_count = bitdescr[chiptype].row_count;
  unsigned row = addr->row;

  row += 1;
  if (row == row_count) {
    row = 0;
    _far_increment_tb(addr);
  }

  addr->row = row;
}

static inline void
_far_increment_col(bitstream_parser_t *bitstream,
		   sw_far_v4_t *addr) {
  const id_t chiptype = bitstream->type;
  const unsigned *col_count = bitdescr[chiptype].col_count;
  const v4_col_type_t type = addr->type;
  guint col;

  col = addr->col;
  col += 1;

  /* There are two pad columns */
  if (col == col_count[type] + 1) {
    col = 0;
    _far_increment_row(bitstream, addr);
  }

  /* writeback the col value */
  addr->col = col;
}

static inline gboolean
_far_is_pad(const bitstream_parser_t *bitstream,
	    const sw_far_v4_t *addr) {
  const id_t chiptype = bitstream->type;
  const unsigned *col_count = bitdescr[chiptype].col_count;
  const v4_col_type_t type = addr->type;
  unsigned col = addr->col;

  if (col >= col_count[type])
    return TRUE;
  return FALSE;
}

/* Type is a bit strange -- it watches the type from the far and the col
   count, so as to get a mixed type which depends on both these
   parameters. */

static inline v4_design_col_t
_type_of_far(bitstream_parser_t *bitstream, const sw_far_v4_t *addr) {
  const id_t chiptype = bitstream->type;
  const v4_col_type_t type = addr->type;

  if (_far_is_pad(bitstream, addr))
    return V4C_PAD;

  switch (type) {
  case V4_TYPE_CLB:
    {
      const int col = addr->col;
      const unsigned end = bitdescr[chiptype].col_count[V4_TYPE_CLB] - 1;
      const unsigned middle = end >> 1;
      const unsigned dsp = end > 50 ? 9 : 12;
      /* Let's be more intelligent.
	 Middle, extremities: IO.
	 DSP is at fixed position and the rest is CLB */
      if (col == 0 || col == end || col == middle)
	return V4C_IOB;
      if (col == (middle + 1))
	return V4C_GCLK;
      if (col == dsp)
	return V4C_DSP48;
      return V4C_CLB;
    }
  case V4_TYPE_BRAM:
    return V4C_BRAM;
  case V4_TYPE_BRAM_INT:
    return V4C_BRAM_INT;
  case V4_TYPE_CFG_CLB:
  case V4_TYPE_CFG_BRAM:
    g_print("Unknown frame type, please report your bitstream");
  default:
    g_assert_not_reached();
  }

  return -1;
}

static inline void
_far_increment_mna(bitstream_parser_t *bitstream,
		   sw_far_v4_t *addr) {
  const id_t chiptype = bitstream->type;
  const int *frame_count = bitdescr[chiptype].frame_count;
  const v4_design_col_t col_type = _type_of_far(bitstream, addr);
  unsigned mna = addr->mna;

  mna += 1;

  if (mna == frame_count[col_type]) {
    mna = 0;
    _far_increment_col(bitstream, addr);
  }

  addr->mna = mna;
}

static inline void
far_increment_mna(bitstream_parser_t *bitstream) {
  sw_far_v4_t far;
  fill_swfar_v4(&far, register_read(bitstream, FAR));
  _far_increment_mna(bitstream, &far);
  register_write(bitstream, FAR, get_hwfar_v4(&far));
}

static inline void
far_increment(bitstream_parser_t *parser) {
  far_increment_mna(parser);
  print_far(parser);
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
	 it as a side-effect */
      break;
    case LOUT: {
      gchar far_name[32];
      snprintf_far_v4(far_name, sizeof(far_name), val);
      g_print("LOUT: %08x ", val);
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
  const char *dataframe = bitstream->last_frame;
  frame_record_t framerec;
  framerec.far = myfar;
  framerec.framelen = 41;
  framerec.frame = dataframe;
  /* record the framerec */
  g_array_append_val(parsed->frame_array, framerec);
}

static void
alloc_indexer(bitstream_parsed_t *parsed) {
  /* */
  parsed->frame_array = g_array_new(FALSE, FALSE, sizeof(frame_record_t));
}

static inline void
free_indexer(bitstream_parsed_t *parsed) {
  GArray *frames = parsed->frame_array;
  if (frames)
    g_array_free(frames, TRUE);
}

void
iterate_over_frames(const bitstream_parsed_t *parsed,
		    frame_iterator_t iter, void *itdat) {
  return;
}

void
iterate_over_unk_frames(const bitstream_parsed_t *parsed,
			frame_unk_iterator_t iter, void *itdat) {
  GArray *array = parsed->frame_array;
  guint nframes = array->len, i;

  /* Iterate over the whole thing */
  for (i = 0; i < nframes; i++) {
    frame_record_t *frame;
    frame = &g_array_index (array, frame_record_t, i);
    iter(frame, itdat);
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
  last_far = register_read(parser, FAR);

  for (i = 0; i < nrframes; i++) {

    /* The first write of a FDRI write in WCFG mode does not flush the
       previous writes. As I don't know what other modes may be on, be
       conservative wrt to mode setting */
    if (i != 0)
      /* flush the previous frame into the frame array with the previous
	 FAR address */
      record_frame(parsed, parser, last_far);

    last_far = register_read(parser, FAR);
    parser->last_frame = frame;

    far_increment(parser);
    frame += frame_len * sizeof(guint32);
  }

  debit_log(L_BITSTREAM,"%i frames written to fdri", i);

  return length;
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
    debit_log(L_BITSTREAM,"execution of %i:%s is a noop",
	      cmd, cmd_names[cmd]);
    break;
  }

  return 0;

}

static gint
idcode_write(bitstream_parsed_t *parsed,
	     bitstream_parser_t *parser) {
  guint32 idcode = register_read(parser, IDCODE);
  int i;

  for (i = 0; i < XC4VLX__NUM; i++)
    if (bitdescr[i].idcode == idcode) {
      parser->type = i;
      parsed->chip = i;
      parsed->chip_struct = &bitdescr[i];
      /* Allocate control structures */
      alloc_indexer(parsed);
      return 0;
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
      switch (pkt) {
      case NOOP:
	debit_log(L_BITSTREAM,"Got NOOP packet");
	return offset;
      case NULLPKT:
	debit_log(L_BITSTREAM,"Null packet while in state %i", state);
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
	/* no AutoCRC processing in v4 */
	break;
      case FAR:
	print_far(parser);
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
