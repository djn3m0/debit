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

#include <string.h>
#include <stdio.h>

#include <glib.h>

#include "bitarray.h"
#include "bitstream_packets.h"
#include "design.h"
#include "bitheader.h"
#include "bitstream_parser.h"
#include "debitlog.h"

#include "codes/crc32-c.h"
#include "codes/xhamming.h"

#if defined(VIRTEX4)
#include "_far_v4.h"
#elif defined(VIRTEX5)
#include "_far_v5.h"
#endif

/*****
 * Common parsing code for virtex4 and virtex5 bitstreams
 */

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
  bcc = crc32c_byte(bcc, val);
  bcc = crc32c_byte(bcc, val >> 8);
  bcc = crc32c_byte(bcc, val >> 16);
  bcc = crc32c_byte(bcc, val >> 24);

  /* the 5 bits of register address */
  bcc = crc32c_addr5(bcc, reg);

  /* write the CRC to the CRC register */
  crcreg->value = bcc;

  /* writes to the CRC should yield a zero value.
     In case of strict checks, this should abort the parsing. */
  if (reg == CRC) {
    debit_log(L_BITSTREAM,"write to CRC register yielded %04x", bcc);
  }
}

/***
 * FAR handling
 */

void
typed_frame_name(char *buf, unsigned buf_len,
		 const unsigned type,
		 const unsigned index,
		 const unsigned frameid) {
  (void) buf; (void) buf_len; (void) type; (void) index; (void) frameid;
}

static inline void
print_far(bitstream_parser_t *parser) {
  const guint32 far = register_read(parser, FAR);
  gchar far_name[32];
  snprintf_far(far_name, sizeof(far_name), far);
  debit_log(L_BITSTREAM, "FAR is [%08x], %s", far, far_name);
}

static inline void
_far_increment_type(sw_far_t *addr) {
  addr->type++;
}

static inline void
_far_increment_row(bitstream_parser_t *bitstream,
		   sw_far_t *addr) {
  const id_vlx_t chiptype = bitstream->type;
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
		   sw_far_t *addr) {
  const id_vlx_t chiptype = bitstream->type;
  const unsigned *col_count = bitdescr[chiptype].col_count;
  const col_type_t type = addr->type;
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
_last_frame(const id_vlx_t chiptype,
	    const sw_far_t *addr) {
  const unsigned *col_count = bitdescr[chiptype].col_count;
  const col_type_t type = addr->type;
  unsigned col = addr->col;

  /* There are pad frames for all three types of data frames */
  if (col >= col_count[type] &&
      type == LAST_COL_TYPE)
    return TRUE;

  return FALSE;
}

static inline gboolean
_far_is_pad(const id_vlx_t chiptype,
	    const sw_far_t *addr) {
  const unsigned *col_count = bitdescr[chiptype].col_count;
  const col_type_t type = addr->type;
  unsigned col = addr->col;

  /* There are pad frames for all three types of data frames */
  if (col >= col_count[type])
    return TRUE;
  return FALSE;
}

static inline gboolean
far_is_pad(bitstream_parser_t *bitstream, guint32 myfar) {
  const id_vlx_t chiptype = bitstream->type;
  sw_far_t far;
  fill_swfar(&far, myfar);
  return _far_is_pad(chiptype, &far);
}

static inline void
_far_increment_mna(bitstream_parser_t *bitstream,
		   sw_far_t *addr) {
  const id_vlx_t chiptype = bitstream->type;
  const unsigned *frame_count = bitdescr[chiptype].frame_count;
  const design_col_t col_type = _type_of_far(chiptype, addr);
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
  sw_far_t far;
  fill_swfar(&far, register_read(bitstream, FAR));
  _far_increment_mna(bitstream, &far);
  register_write(bitstream, FAR, get_hwfar(&far));
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

  debit_log(L_BITSTREAM,"Writing %zd words to register %s",
	    length, reg_names[reg]);

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

static inline const gchar **
get_frameloc_from_swfar(const bitstream_parsed_t *parsed,
			const id_vlx_t chiptype,
			const sw_far_t *far) {
  const design_col_t type = _type_of_far(chiptype, far);
  const unsigned typed_col = _typed_col_of_far(chiptype, far);
  return get_frame_loc(parsed, type, far->row, far->tb, typed_col, far->mna);
}

static inline const gchar **
get_frameloc_from_far(const bitstream_parsed_t *parsed,
		      const id_vlx_t chiptype,
		      const guint32 myfar) {
  sw_far_t far;
  fill_swfar(&far, myfar);
  return get_frameloc_from_swfar(parsed, chiptype, &far);
}

static
void record_frame(bitstream_parsed_t *parsed,
		  bitstream_parser_t *bitstream,
		  const guint32 myfar) {
  const char *dataframe = bitstream->last_frame;
  frame_record_t framerec;
  framerec.far = myfar;
  framerec.framelen = frame_length;
  framerec.frame = dataframe;

  /* Check the frame's Hamming Code */
  /* (void) check_hamming_frame(dataframe, myfar); */

  /* record the framerec, iif the frame is not a pad frame, as pad
     frames are not present in compressed bitstreams, it seems... */
  if (far_is_pad(bitstream, myfar) == FALSE)
    g_array_append_val(parsed->frame_array, framerec);

  /* record the frame in the flat descriptor */
  {
    const id_vlx_t chiptype = bitstream->type;
    const gchar **framepos = get_frameloc_from_far(parsed, chiptype, myfar);
    if (*framepos)
      g_warning("Replacing frame already present for far [%08x]", myfar);
    *framepos = dataframe;
  }
}

/* Bitstream frame indexing */

static unsigned
frames_of_type(const chip_struct_t *chip_struct,
	       const design_col_t type) {
  const unsigned *col_count = chip_struct->col_count;
  return 2 * chip_struct->row_count * type_col_count(col_count,type) * frame_count_v[type];
}

static void
alloc_indexer(bitstream_parsed_t *parsed) {
  const chip_struct_t *chip_struct = parsed->chip_struct;
  gsize total_size = 0;
  gsize type_offset = 0;
  design_col_t type;
  const gchar ***type_lut, **frame_array;

  /* The frame array is a triple-lookup array:
     - first index is index type (design_col_t, length VC__NB_CFG)
     - second index is y-location (length 2*rows)
     - third index is x-location (complex length, to be computed from the frame type)
     - fourth index is mna sublocation (complex length too, from the v4_frame_count)

     The array is indexed on the first index.
  */

  /* We need room for control of the type lookup */
  total_size += VC__NB_CFG * sizeof(gchar **);

  /* We need room for the frames themselves */
  for (type = 0; type < VC__NB_CFG; type++)
    total_size += frames_of_type(chip_struct, type) * sizeof(gchar *);

  /* We allocate only one big array with the type_lut at the beginning
     and the frame_array at the end */
  type_lut = g_new0(const gchar **, total_size);
  frame_array = (const gchar **) &type_lut[VC__NB_CFG];

  /* fill in the control data */
  for (type = 0; type < VC__NB_CFG; type++) {
    type_lut[type] = &frame_array[type_offset];
    type_offset += frames_of_type(chip_struct, type);
  }

  parsed->frames = type_lut;
  parsed->frame_array = g_array_new(FALSE, FALSE, sizeof(frame_record_t));
}

static inline void
free_indexer(bitstream_parsed_t *parsed) {
  GArray *frames = parsed->frame_array;
  void *type_lut = parsed->frames;

  if (type_lut)
    g_free(type_lut);
  if (frames)
    g_array_free(frames, TRUE);
}

void
iterate_over_frames(const bitstream_parsed_t *parsed,
		    frame_iterator_t iter, void *itdat) {
  const id_vlx_t chiptype = 5; /* XC4VLX100 */
  sw_far_t far;
  (void) parsed;
  fill_swfar(&far, 0);

  /* Iterate over the whole thing very dumbly */
  while (!_last_frame(chiptype, &far)) {
/*     const int type = _type_of_far(chip_id, &far); */
/*     const int index = _col_of_far(chip_id, &far); */
/*     const int frame = far.mna; */
    const gchar *data = *get_frameloc_from_swfar(parsed, chiptype, &far);
    iter(data, 0, 0, 0, itdat);
    //_far_increment_mna(chiptype, &far);
  }

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

/* Iterate over frames in FAR-ordered mode. This is a bit complex... */
void
iterate_over_frames_far(const bitstream_parsed_t *parsed,
			frame_iterator_t iter, void *itdat) {
  (void) parsed;
  (void) iter;
  (void) itdat;
}

static gint
handle_fdri_write(bitstream_parsed_t *parsed,
		  bitstream_parser_t *parser,
		  const unsigned length) {
  bytearray_t *ba = &parser->ba;
  const gchar *frame = bytearray_get_ptr(ba);
  guint i, nrframes;
  guint32 last_far = 0;

  /* Frame length writes must be a multiple of the flr length */
  if (length % frame_length) {
    debit_log(L_BITSTREAM,"%i bytes in FDRI write, "
	      "which is inconsistent with the FLR value %i",
	      length, frame_length);
    return -1;
  }

  nrframes = length / frame_length;

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
    frame += frame_length * sizeof(guint32);
  }

  debit_log(L_BITSTREAM,"%i frames written to fdri", i);

  return length;
}

static gint
handle_cmd_write(bitstream_parsed_t *parsed,
		 bitstream_parser_t *parser) {
  cmd_code_t cmd = register_read(parser, CMD);

  switch(cmd) {
  case MFW:
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

  for (i = 0; i < XC_VLX__NUM; i++)
    if (bitdescr[i].idcode == idcode) {
      parser->type = i;
      parsed->chip_struct = &bitdescr[i];
      /* Allocate control structures */
      alloc_indexer(parsed);
      return 0;
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

static gint
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
	/* no AutoCRC processing */
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

#include "bitstream_high.h"
