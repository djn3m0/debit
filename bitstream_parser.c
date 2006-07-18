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

typedef struct _chip_descr {
  guint32 idcode;
  const int col_count[V2C__NB_CFG];
  const int frame_count[V2C__NB_CFG];
} chip_descr_t;

/* typedef enum _idcode { */
/*   XC2V40   = 0x01008093U, */
/*   XC2V80   = 0x01010093U, */
/*   XC2V250  = 0x01018093U, */
/*   XC2V500  = 0x01020093U, */
/*   XC2V1000 = 0x01028093U, */
/*   XC2V1500 = 0x01030093U, */
/*   XC2V2000 = 0x01038093U, */
/*   XC2V3000 = 0x01040093U, */
/*   XC2V4000 = 0x01050093U, */
/*   XC2V6000 = 0x01060093U, */
/*   XC2V8000 = 0x01070093U, */
/* } idcode_t; */

typedef enum _id {
  XC2V40 = 0, XC2V80,
  XC2V250, XC2V500,
  XC2V1000, XC2V1500,
  XC2V2000, XC2V3000,
  XC2V4000, XC2V6000,
  XC2V8000,
  XC2__NUM,
} id_t;

/* This is nothing but a key-value file */

static const chip_descr_t bitdescr[XC2__NUM] = {
  [XC2V2000] = {
    .idcode = 0x01038093U,
    /* used for MJA */
    .col_count = {
      [V2C_IOB] = 2,
      [V2C_IOI] = 2,
      [V2C_CLB] = 48,
      [V2C_BRAM] = 4,
      [V2C_BRAM_INT] =  4,
      [V2C_GCLK] = 1,
    },
    /* used for MNA */
    .frame_count = {
      [V2C_IOB] = 4,
      [V2C_IOI] = 22,
      [V2C_CLB] = 22,
      [V2C_BRAM] = 64,
      [V2C_BRAM_INT] =  22,
      [V2C_GCLK] = 4,
    },
    /* FLR value */
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

  /* Specific quirks */
  guint32 last_far;
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
  //  g_warning("CRC now %04x", bcc);
}

/***
 * FAR
 ***/

static inline void
print_far(sw_far_t *far) {
  g_warning("FAR is [ba %i, mja %i, mna %i, bn %i]",
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
    g_warning("Unrecognized ba type %i",ba);
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

  g_warning("Writing %i words to register %i", length, reg);

  for (i = 0; i < length; i++) {
    guint32 val = bytearray_get_uint32(ba);
    regp->value = val;
    update_crc(parser, reg, val);
  }
  parser->active_length -= length;

}

static inline
void record_frame(bitstream_parsed_t *parsed,
		  bitstream_parser_t *bitstream) {
  sw_far_t far;
  guint type, index, frame;

  if (!bitstream->last_frame)
    return;

  fill_swfar(&far, bitstream->last_far);

  type = _type_of_far(bitstream, &far);
  index = _col_of_far(bitstream, &far);
  frame = far.mna;

  g_warning("flushing frame [type:%i,index:%02i,frame:%2X]",
	    type, index, frame);
  *get_frame_loc(parsed, type, index, frame) = bitstream->last_frame;
}

static void
alloc_indexer(bitstream_parsed_t *parsed) {
  gsize total_size = 0;
  gsize type_offset = 0;
  gsize type;
  const gchar ***type_lut, **frame_array;

  /* We need room for the type lookup */
  total_size += V2C__NB_CFG * sizeof(gchar **);

  /* NB: We don't memoize the column lookup -- we prefer a multiplication
     for this  */
  for (type = 0; type < V2C__NB_CFG; type++)
    total_size += v2_frame_count[type] * v2_col_count[type] * sizeof(gchar *);

  /* We allocate only one big array with the type_lut at the beginning
     and the frame_array at the end */
  type_lut = g_new0(const gchar **, total_size);
  frame_array = (const gchar **) &type_lut[V2C__NB_CFG];

  /* fill in the control data */
  for (type = 0; type < V2C__NB_CFG; type++) {
    type_lut[type] = &frame_array[type_offset];
    type_offset += v2_col_count[type] * v2_frame_count[type];
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
  guint type, index, frame;

  /* Iterate over the whole thing */
  for (type = 0; type < V2C__NB_CFG; type++) {
    const guint col_count = v2_col_count[type];

    for (index = 0; index < col_count; index++) {
      const guint frame_count = v2_frame_count[type];

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
		  unsigned length) {
  bytearray_t *ba = &parser->ba;
  const guint frame_len = frame_length(parser);
  gint offset;

  /* NB: loop here */
  offset = frame_len;

  if (length < frame_len) {
    g_warning("%i bytes remaining in FDRI write, "
	      "which is inconsistent with the FLR value %i",
	      length, frame_len);
    return -1;
  }

  /* flush the previous frame into the frame array with the previous FAR
     address */
  record_frame(parsed, parser);

  parser->last_far = register_read(parser, FAR);
  parser->last_frame = bytearray_get_ptr(ba);

  far_increment_mna(parser);

  /* AutoCRC and flushing frame at the end of a FDRI write */
  if (length == frame_len) {
    parser->last_frame = NULL;
    offset += 1;
  }

  return offset;
}

int synchronize_bitstream(bitstream_parser_t *parser) {
  bytearray_t *ba = &parser->ba;

  /* XXX guint32 data access must be aligned -- make sure it is */
  /* advance the bitstream until the sync word is found */
  while (bytearray_peek_uint32(ba) != SYNCHRO) {
    (void) bytearray_get_uint32(ba);
  }

  g_assert(bytearray_get_uint32(ba) == SYNCHRO);
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
    g_warning("Could not synchronize bitstream");
    return err;
  }

  /* Then launch the computation */
  do {
    advance = read_next_token(dest, parser);
  } while(advance > 0);

  if (advance < 0)
    g_warning("Error parsing bitstream: %i", advance);

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
    g_warning("header parsing error");
    return -1;
  }

  /* Do some allocations according to the type of the bitfile */
  bytearray_init(&parser.ba, len, offset, buf_in);
  /* XXX use idcode instead */
  parser.type = XC2V2000;
  alloc_indexer(dest);

  return _parse_bitstream_data(dest, &parser);
}

static inline void
print_parser_state(const bitstream_parser_t *parser) {
  gint state = parser->state;
  switch(state) {
  case STATE_WAITING_CTRL:
    g_warning("Waiting CTRL");
    break;
  case STATE_WAITING_DATA:
    g_warning("Waiting DATA, %i words remaining for register %i",
	      parser->active_length, parser->active_register);
    break;
  default:
    g_warning("Unknown parser state %i",state);
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

  print_parser_state(parser);

  switch(state) {
  case STATE_WAITING_CTRL:
    {
      gsize avail = bytearray_available(ba);
      guint32 pkt;

      /* For now we don't error out in this state */
      if (avail == 0) {
	g_warning("End-of-bitstream reached");
	return 0;
      }

      pkt = bytearray_get_uint32(ba);

      /* catch a noop */
      if (pkt == NOOP) {
	g_warning("Got NOOP packet");
	return offset;
      }

      /* v1 or v2 packet */
      switch (type_of_pkt1(pkt)) {
      case V1: {
	g_warning("Got V1 packet");
	parser->active_register = rega_of_pkt1(pkt);
	parser->active_length = wordc_of_pkt1(pkt);
	parser->write__read = wr_of_pkt1(pkt);
	break;
      }
      case V2: {
	g_warning("Got V2 packet");
	parser->active_length = wordc_of_v2pkt(pkt);
	break;
      }
      case NOOP:
	break;

      default:
	g_warning("Unrecognized packet %08x while in state %i", pkt, state);
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

      if (offset > avail) {
	g_warning("Register length of %i words while only %i words remain",
		  offset, avail);
	return -1;
      }

      offset = length;

      switch(reg) {
      case FDRI:
	g_warning("FDRI setting");
	offset = handle_fdri_write(parsed, parser, length);
	break;
      case FAR:
	g_warning("FAR setting");
	break;
      case FLR:
	g_warning("FLR setting");
	break;
      default:
	break;
      }

      default_register_write(parser, reg, offset);

      /* < 0 happens with autocrc on FDRI writes. We must then check
	 that the CRC is alright */
      if (parser->active_length == 0)
	parser->state = STATE_WAITING_CTRL;
      if (parser->active_length < 0) {
	g_warning("AutoCRC is %04x", parser->registers[FDRI].value);
	parser->state = STATE_WAITING_CTRL;
      }

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

  if (error != NULL) {
    g_warning("could not map file %s: %s",filename,error->message);
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
  g_free(bitstream);
  return;
}
