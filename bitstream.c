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

#include <assert.h>
#include <glib.h>
#include "bitstream.h"
#include "design.h"
#include "cfgbit.h"
#include "bitstream_parser.h"
#include "virtex2_config.h"

/** \file
 *
 * This file is the interface to use to access the data in the
 * bitstream. Some of the facilities programmed here should be
 * abstracted from the bitstream type (currently xc2v2000) with an
 * appropriate database. What form this database should take remains to
 * be seen.
 *
 * There are two kinds of functions here. Untyped query functions, and
 * typed query functions. Typed query functions are typically wrappers
 * to untyped query functions. Typed query functions are also present in
 * localpips.c
 *
 * We should probably go from a bit query to a byte query (of up to 4
 * bytes, or one guint32).
 */

/**
 * Describes the layout of the configuration bits of a site in the bitstream.
 */

#if defined(VIRTEX2)

#define CLB_HEIGHT 10

const type_bits_t type_bits[NR_SITE_TYPE] = {
  /* CLB Group */
  [CLB] = {
    .col_type = V2C_CLB,
    .x_type_off = 0,
    .y_offset = CLB_HEIGHT + 2,
    .y_width = CLB_HEIGHT,
  },
  [LTERM] = {
    .col_type = V2C_IOB,
    .x_type_off = 0,
    .y_offset = CLB_HEIGHT + 2,
    .y_width = CLB_HEIGHT,
  },
  [RTERM] = {
    .col_type = V2C_IOB,
    .x_type_off = 1,
    .y_offset = CLB_HEIGHT + 2,
    .y_width = CLB_HEIGHT,
  },
  [LIOI] = {
    .col_type = V2C_IOI,
    .x_type_off = 0,
    .y_offset = CLB_HEIGHT + 2,
    .y_width = CLB_HEIGHT,
  },
  [RIOI] = {
    .col_type = V2C_IOI,
    .x_type_off = 1,
    .y_offset = CLB_HEIGHT + 2,
    .y_width = CLB_HEIGHT,
  },
  [TTERM] = {
    .col_type = V2C_CLB,
    .x_type_off = 0,
    .y_offset = 0,
    .y_width = 2,
  },
  [BTERM] = {
    .col_type = V2C_CLB,
    .x_type_off = 0,
    .y_offset = -2,
    .y_width = 2,
  },
  [TIOI] = {
    .col_type = V2C_CLB,
    .x_type_off = 0,
    .y_offset = 2,
    .y_width = CLB_HEIGHT,
  },
  [BIOI] = {
    .col_type = V2C_CLB,
    .x_type_off = 0,
    .y_offset = - ((int)CLB_HEIGHT + 2),
    .y_width = CLB_HEIGHT,
  },
  /* BRAM Group */
  [BRAM] = {
    .col_type = V2C_BRAM_INT,
    .x_type_off = 0,
    .y_offset = CLB_HEIGHT + 2,
    .y_width = CLB_HEIGHT,
  },
  [TTERMBRAM] = {
    .col_type = V2C_BRAM_INT,
    .x_type_off = 0,
    .y_offset = 0,
    .y_width = 2,
  },
  [BTERMBRAM] = {
    .col_type = V2C_BRAM_INT,
    .x_type_off = 0,
    .y_offset = -2,
    .y_width = 2,
  },
  [TIOIBRAM] = {
    .col_type = V2C_BRAM_INT,
    .x_type_off = 0,
    .y_offset = 2,
    .y_width = CLB_HEIGHT,
  },
  [BIOIBRAM] = {
    .col_type = V2C_BRAM_INT,
    .x_type_off = 0,
    .y_offset = - ((int)CLB_HEIGHT + 2),
    .y_width = CLB_HEIGHT,
  },
  /* Still todo:
     these nothing:
       TLTERM, LTTERM, LBTERM, BLTERM, BRTERM, RBTERM, RTTERM, TRTERM,
     this one nothing:
       BM,
     these unknown:
       TL, BL, BR, TR, M,
       these shitty, will need something specific:
     GCLKH, GCLKHBRAM,
  */
  /*
  [CLKT] = {
    .col_type = V2C_GCLK,
    .x_type_off = 0,
    .y_offset = - (),
    .y_width = CLB_HEIGHT,
  },
  [CLKB] = {
    .col_type = V2C_GCLK,
    .x_type_off = 0,
    .y_offset = ,
    .y_width = CLB_HEIGHT,
  },
  [GCLKC] = {
    .col_type = V2C_GCLK,
    .x_type_off = 0,
    .y_offset = ,
    .y_width = CLB_HEIGHT,
  },
  */
  /* GCLKH are harder */
};

#elif defined(SPARTAN3) /* VIRTEX2 */

#define CLB_HEIGHT 8

const type_bits_t type_bits[NR_SITE_TYPE] = {
  /* CLB Group */
  [CLB] = {
    .col_type = V2C_CLB,
    .x_type_off = 0,
    .y_offset = CLB_HEIGHT + 2,
    .y_width = CLB_HEIGHT,
  },
  [LTERM] = {
    .col_type = V2C_IOB,
    .x_type_off = 0,
    .y_offset = CLB_HEIGHT + 2,
    .y_width = CLB_HEIGHT,
  },
  [RTERM] = {
    .col_type = V2C_IOB,
    .x_type_off = 1,
    .y_offset = CLB_HEIGHT + 2,
    .y_width = CLB_HEIGHT,
  },
  [LIOI] = {
    .col_type = V2C_IOI,
    .x_type_off = 0,
    .y_offset = CLB_HEIGHT + 2,
    .y_width = CLB_HEIGHT,
  },
  [RIOI] = {
    .col_type = V2C_IOI,
    .x_type_off = 1,
    .y_offset = CLB_HEIGHT + 2,
    .y_width = CLB_HEIGHT,
  },
  [TTERM] = {
    .col_type = V2C_CLB,
    .x_type_off = 0,
    .y_offset = 0,
    .y_width = 2,
  },
  [BTERM] = {
    .col_type = V2C_CLB,
    .x_type_off = 0,
    .y_offset = -2,
    .y_width = 2,
  },
  [TIOI] = {
    .col_type = V2C_CLB,
    .x_type_off = 0,
    .y_offset = 2,
    .y_width = CLB_HEIGHT,
  },
  [BIOI] = {
    .col_type = V2C_CLB,
    .x_type_off = 0,
    .y_offset = - (CLB_HEIGHT + 2),
    .y_width = CLB_HEIGHT,
  },
  /* BRAM Group */
  [BRAM] = {
    .col_type = V2C_BRAM_INT,
    .x_type_off = 0,
    .y_offset = CLB_HEIGHT + 2,
    .y_width = CLB_HEIGHT,
  },
  [TTERMBRAM] = {
    .col_type = V2C_BRAM_INT,
    .x_type_off = 0,
    .y_offset = 0,
    .y_width = 2,
  },
  [BTERMBRAM] = {
    .col_type = V2C_BRAM_INT,
    .x_type_off = 0,
    .y_offset = -2,
    .y_width = 2,
  },
  [TIOIBRAM] = {
    .col_type = V2C_BRAM_INT,
    .x_type_off = 0,
    .y_offset = 2,
    .y_width = CLB_HEIGHT,
  },
  [BIOIBRAM] = {
    .col_type = V2C_BRAM_INT,
    .x_type_off = 0,
    .y_offset = - (CLB_HEIGHT + 2),
    .y_width = CLB_HEIGHT,
  },
};

#endif /* SPARTAN3 */

/*
 * Untyped query functions
 */


/** \brief Get one config byte from a site
 *
 * @param bitstream the bitstream data
 * @param site the site queried
 * @param cfgbyte the bit asked for
 *
 * @return the configuration byte asked for
 */

static const gchar *
query_bitstream_site_bytea(const bitstream_parsed_t *bitstream,
			   const csite_descr_t *site,
			   const unsigned cfgbit) {
  const chip_struct_t *chip_struct = bitstream->chip_struct;
  const guint site_type = site->type;
  const guint x = site->type_coord.x;
  const guint y = site->type_coord.y;
  const guint y_width = type_bits[site_type].y_width;
  const guint flen = chip_struct->framelen * sizeof(uint32_t);
  const gint y_offset = type_bits[site_type].y_offset;

  /* site offset in the y axis -- inverted. Should not be done here maybe */
  const guint y_type_offset = (y_offset >= 0) ? (unsigned)y_offset : (flen + y_offset);
  const off_t site_off = y * y_width + y_type_offset;

  /* offset in-site. only this really needs to be computed locally */
  const guint xoff = byte_x(cfgbit);
  const guint yoff = byte_y(cfgbit);
  /* The database could be changed to have only one add here */
  const gsize frame_offset = site_off + yoff;

  const gchar *frame = get_frame(bitstream, type_bits[site_type].col_type,
				 x+type_bits[site_type].x_type_off, xoff);

/*   debit_log(L_FILEPOS, "querying site t%u, (%u,%u), cfgbit %u", */
/* 	    site->type, site->type_coord.x, site->type_coord.y, cfgbit); */

  return &frame[frame_offset];
}

static inline guchar
query_bitstream_site_byte(const bitstream_parsed_t *bitstream,
			  const csite_descr_t *site,
			  const unsigned cfgbit) {
  return *query_bitstream_site_bytea(bitstream, site, cfgbit);
}

/** \brief Get some (up to 4) config bytes from a site
 *
 * @param bitstream the bitstream data
 * @param site the site queried
 * @param cfgbits the array of bytes asked for
 * @param nbytes the number of bytes asked for
 *
 * @return the configuration bytes asked for, packed into a guint32
 * @see query_bitstream_site_byte
 */

static guint32
query_bitstream_site_bytes(const bitstream_parsed_t * bitstream, const csite_descr_t *site,
			   const guint *cfgbytes, const gsize nbytes) {
  guint32 result = 0;
  gsize i;

  for(i = 0; i < nbytes; i++)
    result |= query_bitstream_site_byte(bitstream, site, cfgbytes[i]) << (i<<3);

  return result;
}

/** \brief Get one config bit from a site
 *
 * @param bitstream the bitstream data
 * @param site the site queried
 * @param cfgbit the bit asked for
 *
 * @return the configuration bit asked for
 * @see query_bitstream_site_byte
 */

static inline gboolean
query_bitstream_site_bit(const bitstream_parsed_t *bitstream,
			 const csite_descr_t *site,
			 const guint cfgbit) {
  const guchar cfgbyte_dat = query_bitstream_site_byte(bitstream, site, cfgbit);
  const guint  cfgbyte_off = bit_offset(cfgbit);

  if ((cfgbyte_dat >> cfgbyte_off) & 1)
    return TRUE;

  return FALSE;
}

/** \brief Get some (up to 32) config bits from a site
 *
 * @param bitstream the bitstream data
 * @param site the site queried
 * @param cfgbits the array of bits asked for
 * @param nbits the number of bits asked for
 *
 * @return the configuration bits asked for, packed into a guint32
 *
 */

guint32
query_bitstream_site_bits(const bitstream_parsed_t * bitstream,
			  const csite_descr_t *site,
			  const guint *cfgbits, const gsize nbits) {
  /* unsigned char *data = bitstream->bincols[pip_type]; */
  /* The data has already been sliced according to its type. It is not
     obvious that this is what we should do */
  unsigned last_byte = -1;
  uint32_t last_byte_val = 0;
  guint32 result = 0;
  unsigned i;

  /* XXX This mechanism could, should be pushed down to the database
     format */
  for(i = 0; i < nbits; i++) {
    unsigned cfgbit = cfgbits[i];
    unsigned byteaddr = byte_addr(cfgbit);
    unsigned byteoff  = bit_offset(cfgbit);

    /* Get the new byte if necessary */
    if (last_byte != byteaddr)
      last_byte_val = query_bitstream_site_byte(bitstream, site, byteaddr);

    /* Then do the rest of the query */
    last_byte = byteaddr;
    result |= (((last_byte_val >> byteoff) & 1)) << i;
  }

  return result;
}

/* Bitstream bit setting function. The value to be set is encoded as
   the LSB of the address in cfgbits.
   XXX Optimize calls to query_bitstream_site_bytea and mask/val
   manipulations by regrouping byte lookups.
*/

void set_bitstream_site_bits(const bitstream_parsed_t * bitstream,
			     const csite_descr_t *site,
			     const uint32_t vals,
			     const guint cfgbits[], const gsize nbits) {
  unsigned i;

  for (i = 0; i < nbits; i++) {
    unsigned cfgbit = cfgbits[i];
    unsigned val = (vals >> i) & 1;
    unsigned byteaddr = byte_addr(cfgbit);
    unsigned byteoff  = bit_offset(cfgbit);

    /* Readback value - XXX */
    char *bytea = (void *)query_bitstream_site_bytea(bitstream, site, byteaddr);
    unsigned char byte_val = *bytea;

    /* Update the value */
    unsigned char mask = 1 << byteoff;
    unsigned char valm = val << byteoff;

    /* We shouldn't find anybody where we intend to write,
       for now. Unfortunately this is not true, due to BX3 & al
       sucking... to be investigated ! */
    //assert((byte_val & mask) == 0);

    /* Set bits which need to be set */
    byte_val |= mask & valm;
    /* Clear others. Should be a noop for now,
       as we don't want to overwrite anything. */
/*     byte_val &= ~mask | valm; */

    /* Writeback the value */
    *bytea = byte_val;
  }
}


/*
 * Typed queries
 */

/*
 * There's a nice and tidy canonical way to do this...
 */

static inline
guint16 reverse_bits(guint16 input) {
  guint16 res = 0;
  guint i;
  for(i = 0; i < 16; i++)
    if (input & (1<<i))
      res |= 1 << (15-i);
  return res;
}

/** \brief Get the LUT configuration bits from the bitstream
 *
 * @param bitstream the bitstream data
 * @param site the site queried
 * @param luts array of 4 guint16, used to return the four LUT values of
 * the slice
 *
 * @return the configuration bits
 */
void
query_bitstream_luts(const bitstream_parsed_t *bitstream,
		     const csite_descr_t *site, guint16 luts[]) {
  guint i;

  /* query eight luts. Bits are MSB first, but in reverse order */
  for (i=0; i < 8; i++) {
    /* X position of the LUT is only guessed for spartan3 */
#ifdef VIRTEX2
    unsigned byte_lut_offset = bitpos_invert(5 * BITAT(i,1) + 3 * BITAT(i,0), CLB_HEIGHT);
#else /* VIRTEX2 */
    unsigned byte_lut_offset = bitpos_invert(4 * BITAT(i,1) + 2 * BITAT(i,0), CLB_HEIGHT);
#endif /* VIRTEX2 */
    /* Y-position of the LUT is 1+BITAT(i,2) */
    guint first_byte = assemble_cfgbit(1+BITAT(i,2), byte_lut_offset);
    /* minus height, to account for the reverse occuring in
       bitpos_invert */
    guint cfgbytes[2] = { first_byte, first_byte-8 };
    guint32 result;

    result = query_bitstream_site_bytes(bitstream, site, cfgbytes, 2);
    result = ~result;
    /* Mirroring... of G-luts */
    luts[i] = BITAT(i,0) ? result : reverse_bits(result);
  }

  return;
}

void
set_bitstream_lut(const bitstream_parsed_t *bitstream,
		  const csite_descr_t *site,
		  const guint16 lut_val, const unsigned lut_i) {
  /* X position of the LUT is only guessed for spartan3 */
#ifdef VIRTEX2
  unsigned byte_lut_offset = bitpos_invert(5 * BITAT(lut_i,1) + 3 * BITAT(lut_i,0), CLB_HEIGHT);
#else /* VIRTEX2 */
  unsigned byte_lut_offset = bitpos_invert(4 * BITAT(lut_i,1) + 2 * BITAT(lut_i,0), CLB_HEIGHT);
#endif /* VIRTEX2 */
  /* Y-position of the LUT is 1+BITAT(lut_i,2) */
  guint first_byte = assemble_cfgbit(1+BITAT(lut_i,2), byte_lut_offset);
  /* minus height, to account for the reverse occuring in
     bitpos_invert */
  guint cfgbytes[2] = { first_byte, first_byte-8 };
  guint16 write_lut_val = BITAT(lut_i,0) ? ~lut_val : reverse_bits(~lut_val);
  /* XXX */
  char *addr1 = (void *) query_bitstream_site_bytea(bitstream, site, cfgbytes[0]);
  char *addr2 = (void *) query_bitstream_site_bytea(bitstream, site, cfgbytes[1]);

  assert(lut_i < 8);

  *addr1 = write_lut_val & 0xff;
  *addr2 = write_lut_val >> 8;

  return;
}

typedef int property_t;

/*
 * Same kind of interface, for different properties. Merge with above
 * once the config bits are unified.
 */
static guint16 __attribute__((unused))
query_bistream_config(const bitstream_parsed_t *bitstream,
		      const site_t *site, const guint subsite,
		      const property_t *prop) {
  (void) bitstream;
  (void) site;
  (void) subsite;
  (void) prop;
  return 0;
}

#if defined(VIRTEX2)

#define BRAM_FRAMES 64
#define BRAM_WORD_PER_FRAME 16

static const
guchar bram_offset_for_bit[16] = {
  1, 1, 1, 2, 1, 1, 1, 3, 1, 1, 1, 2, 1, 1, 1,
};

#else /* VIRTEX2 */

/* Not really reverse-engineered yet, just a wild guess */

/* SPARTAN3 */
#define BRAM_FRAMES 76
#define BRAM_WORD_PER_FRAME 16

static const
guchar bram_offset_for_bit[16] = {
  1, 1, 1, 2, 1, 1, 1, 3, 1, 1, 1, 2, 1, 1, 1,
};

#endif /* VIRTEX2 */

static const
guchar bram_bit_to_word[16] = {
  6, 4, 2, 0, 8, 10, 12, 14, 15, 13, 11, 9, 1, 3, 5, 7,
};

static const
guint16 bram_offset_to_mask[16] = {
  0x10, 0x800, 0x20, 0x400, 0x40, 0x200, 0x80, 0x100, 0x8, 0x1000, 0x4, 0x2000, 0x2, 0x4000, 0x1, 0x8000,
};

/** \brief Get the bram data bits from a site
 *
 * @param bitstream the bitstream data
 * @param site the site queried
 * @return the bram data array
 */

guint16 *
query_bitstream_bram_data(const bitstream_parsed_t *bitstream,
			  const csite_descr_t *site) {
  /* Actually this is only bit reordering */
  /* for now exctract the data from the bram coordinates ? */
  const guint x = site->type_coord.x;
  /* the bram spans 4 sites in height */
  const guint y = site->type_coord.y >> 2;
  const unsigned bram_width = 4 * CLB_HEIGHT;
  const unsigned site_offset = 2 + CLB_HEIGHT + y * bram_width + (bram_width - 2);

  guint16 *bram_data = g_new0(guint16,64*16);
  guint i,j,k;

  /* iterate over BRAM columns (config line) */
  for (i = 0; i < 64; i++) {
    guint16 *line_data = &bram_data[16*i];
    unsigned guint_offset = site_offset / 2;
    const guint16 *frame = (const guint16 *) get_frame(bitstream, V2C_BRAM, x, i);

    for (j = 0; j < 16; j++) {
      guint16 data = GUINT16_FROM_BE(frame[guint_offset]);
      guint16 bit_to_write = (1 << j);

      for (k = 0; k < 16; k++) {
	if (bram_offset_to_mask[k] & data)
	  line_data[k] |= bit_to_write;
      }

      guint_offset -= bram_offset_for_bit[j];
    }
  }
  return bram_data;
}

/** \brief Get the bram parity bits from a site
 *
 * @param bitstream the bitstream data
 * @param site the site queried
 * @return the bram parity bits array
 */

__attribute__((unused)) static const gchar *
query_bitstream_bram_parity(const bitstream_parsed_t *bitstream,
			    const site_t *site) {
  /* Actually this is only bit reordering */
  (void) bitstream;
  (void) site;
  return NULL;
}

/** \brief Get the total configuration size in bytes for a site type
 *
 * @param bitstream the bitstream data
 * @param site the site queried
 * @return the bram parity bits array
 */
gsize
query_bitstream_type_size(const bitstream_parsed_t *parsed,
			  const site_type_t type) {
  const chip_struct_t *chip_struct = parsed->chip_struct;
  const unsigned *frame_count = chip_struct->frame_count;
  const type_bits_t *type_bit = &type_bits[type];
  return frame_count[type_bit->col_type] * type_bit->y_width;
}

/** \brief Get all configuration bits from a site
 *
 * @param bitstream the bitstream data
 * @param site the site queried
 * @data the data buffer whereto write the data. It must be sufficiently
 * large for receiving the data
 * @return error code
 *
 * @see query_bitstream_type_size
 * @see query_bitstream_byte
 */

int
query_bitstream_site_data(gchar *data, const gsize nbytes,
	                  const bitstream_parsed_t *parsed,
			  const csite_descr_t *site) {
  const unsigned width = type_bits[site->type].y_width;
  gsize i;

  for (i = 0; i < nbytes; i++) {
    unsigned pos = bitpos_to_cfgbit(i << 3, width);
    data[i] = query_bitstream_site_byte(parsed, site, pos);
  }

  return 0;
}
