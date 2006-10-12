/*
 * (C) Copyright 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
 * All rights reserved.
 * Agnostic parser. Used for reverse-engineering.
 */

#include <glib.h>
#include <string.h>

#include "bitarray.h"
#include "design.h"
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

static const
chip_struct_t bitdescr[XC4VLX__NUM] = {
  /* FLR is always 41 for virtex-4 */
  [XC4VLX15] = { .idcode = 0x01658093,
		 .framelen = 41, },
  [XC4VLX25] = { .idcode = 0x0167C093,
		 .framelen = 41, },
  [XC4VLX40] = { .idcode = 0x016A4093,
		 .framelen = 41, },
  [XC4VLX60] = { .idcode = 0x016B4093,
		 .framelen = 41, },
  [XC4VLX80] = { .idcode = 0x016D8093,
		 .framelen = 41, },
  [XC4VLX100] = { .idcode = 0x01700093,
		  .framelen = 41, },
  [XC4VLX160] = { .idcode = 0x01718093,
		  .framelen = 41, },
  [XC4VLX200] = { .idcode = 0x01734093,
		  .framelen = 41, },
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
  xil_register_t registers[__V4_NUM_REGISTERS];

  /* detailed view of some registers */
  id_t type;

  /* Specific FDRI quirks */
  gboolean fdri_direct_mode;
  const void *last_frame;
  unsigned far_offset;

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

static inline void
far_increment(bitstream_parser_t *parser) {
  parser->far_offset++;
}

static inline void
far_offset_reset(bitstream_parser_t *parser) {
  parser->far_offset = 0;
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

static inline void
print_far(bitstream_parser_t *parser) {
  const guint32 far = register_read(parser, FAR);
  const guint32 index = parser->far_offset;
  gchar far_name[32];
  snprintf_far_v4(far_name, sizeof(far_name), far);
  debit_log(L_BITSTREAM, "FAR is [%i, offset %i], %s", far, index, far_name);
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
    /* XXX For v4 this is correct code. Check for others (v2) */
    if (reg != CRC)
      regp->value = val;
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
  framerec.offset = bitstream->far_offset;
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
handle_fdri_write_direct(bitstream_parsed_t *parsed,
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

    /* V4 has no flush frame - yeepee ! */
    parser->last_frame = frame;
    record_frame(parsed, parser, last_far);

    /* evolution of the state machine */
    far_increment(parser);
    frame += frame_len * sizeof(guint32);
  }

  debit_log(L_BITSTREAM,"%i frames written to fdri", i);

  return length;
}

static gint
handle_fdri_write_flush(bitstream_parsed_t *parsed,
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
    if (i != 0) {
      /* flush the previous frame into the frame array with the previous
	 FAR address */
      record_frame(parsed, parser, last_far);
      far_increment(parser);
    }

    parser->last_frame = frame;
    frame += frame_len * sizeof(guint32);
  }

  debit_log(L_BITSTREAM,"%i frames written to fdri", i);

  return length;
}

static gint
handle_fdri_write(bitstream_parsed_t *parsed,
		  bitstream_parser_t *parser,
		  const unsigned length) {
  if (parser->fdri_direct_mode)
    return handle_fdri_write_direct(parsed,parser,length);
  return handle_fdri_write_flush(parsed,parser,length);
}

static void
handle_far_write(bitstream_parser_t *parser) {
  cmd_code_t cmd = register_read(parser, CMD);
  debit_log(L_BITSTREAM,"FAR write resetting the far offset");
  far_offset_reset(parser);
  print_far(parser);
  if (cmd == CMD_NULL) {
    /* This is "direct mode" for FDRI write */
    debit_log(L_BITSTREAM,"FAR write with NULL command activating direct FDRI mode");
    parser->fdri_direct_mode = TRUE;
  } else {
    debit_log(L_BITSTREAM,"FAR write with standard command disabling FDRI mode");
    parser->fdri_direct_mode = FALSE;
  }
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
	/* no AutoCRC processing in v4 */
	break;
      case FAR:
	handle_far_write(parser);
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
