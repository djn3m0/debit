/*
 * (C) Copyright 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
 * All rights reserved.
 */

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

const type_bits_t type_bits[NR_SITE_TYPE] = {
  /* CLB Group */
  [CLB] = {
    .col_type = V2C_CLB,
    .x_type_off = 0,
    .y_offset = sizeof(site_descr_t) + 2,
    .y_width = sizeof(site_descr_t),
    .row_count = SITE_PER_COL,
  },
  [LTERM] = {
    .col_type = V2C_IOB,
    .x_type_off = 0,
    .y_offset = sizeof(site_descr_t) + 2,
    .y_width = sizeof(site_descr_t),
    .row_count = SITE_PER_COL,
  },
  [RTERM] = {
    .col_type = V2C_IOB,
    .x_type_off = 1,
    .y_offset = sizeof(site_descr_t) + 2,
    .y_width = sizeof(site_descr_t),
    .row_count = SITE_PER_COL,
  },
  [LIOI] = {
    .col_type = V2C_IOI,
    .x_type_off = 0,
    .y_offset = sizeof(site_descr_t) + 2,
    .y_width = sizeof(site_descr_t),
    .row_count = SITE_PER_COL,
  },
  [RIOI] = {
    .col_type = V2C_IOI,
    .x_type_off = 1,
    .y_offset = sizeof(site_descr_t) + 2,
    .y_width = sizeof(site_descr_t),
    .row_count = SITE_PER_COL,
  },
  [TTERM] = {
    .col_type = V2C_CLB,
    .x_type_off = 0,
    .y_offset = 0,
    .y_width = 2,
    .row_count = 1,
  },
  [BTERM] = {
    .col_type = V2C_CLB,
    .x_type_off = 0,
    .y_offset = -2,
    .y_width = 2,
    .row_count = 1,
  },
  [TIOI] = {
    .col_type = V2C_CLB,
    .x_type_off = 0,
    .y_offset = 2,
    .y_width = sizeof(site_descr_t),
    .row_count = 1,
  },
  [BIOI] = {
    .col_type = V2C_CLB,
    .x_type_off = 0,
    .y_offset = - ((int)sizeof(site_descr_t) + 2),
    .y_width = sizeof(site_descr_t),
    .row_count = 1,
  },
  /* BRAM Group */
  [BRAM] = {
    .col_type = V2C_BRAM_INT,
    .x_type_off = 0,
    .y_offset = sizeof(site_descr_t) + 2,
    .y_width = sizeof(site_descr_t),
    .row_count = SITE_PER_COL,
  },
  [TTERMBRAM] = {
    .col_type = V2C_BRAM_INT,
    .x_type_off = 0,
    .y_offset = 0,
    .y_width = 2,
    .row_count = 1,
  },
  [BTERMBRAM] = {
    .col_type = V2C_BRAM_INT,
    .x_type_off = 0,
    .y_offset = -2,
    .y_width = 2,
    .row_count = 1,
  },
  [TIOIBRAM] = {
    .col_type = V2C_BRAM_INT,
    .x_type_off = 0,
    .y_offset = 2,
    .y_width = sizeof(site_descr_t),
    .row_count = 1,
  },
  [BIOIBRAM] = {
    .col_type = V2C_BRAM_INT,
    .x_type_off = 0,
    .y_offset = - ((int)sizeof(site_descr_t) + 2),
    .y_width = sizeof(site_descr_t),
    .row_count = 1,
  },
  /* Still todo:
     these nothing:
       TLTERM, LTTERM, LBTERM, BLTERM, BRTERM, RBTERM, RTTERM, TRTERM,
     this one nothing:
       BM,
     these unknown:
       TL, BL, BR, TR,
       M,
     these shitty, will need something specific:
       CLKT, CLKB, GCLKC, GCLKH, GCLKHBRAM,
  */
};

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

static guchar
query_bitstream_site_byte(const bitstream_parsed_t *bitstream,
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
  const guint y_type_offset = (y_offset >= 0) ? y_offset : (flen + y_offset);
  const off_t site_off = y * y_width + y_type_offset;

  /* offset in-site. only this really needs to be computed locally */
  const guint xoff = byte_x(cfgbit);
  const guint yoff = byte_y(cfgbit);
  /* The database could be changed to have only one add here */
  const gsize frame_offset = site_off + yoff;

  const gchar *frame = get_frame(bitstream, type_bits[site_type].col_type,
				 x+type_bits[site_type].x_type_off, xoff);

  return frame[frame_offset];
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

guint32
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
 * \bug For now we use the site as a CLB site, whether I think Eric intended
 * this to be a global, type-independent site.
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

  /* query four luts. Bits are MSB first, but in reverse order */
  for (i=0; i < 4; i++) {
    unsigned byte_lut_offset = bitpos_invert(2 * i + ((i & 2) >> 1), sizeof(site_descr_t));
    guint first_byte = assemble_cfgbit(1, byte_lut_offset);
    guint cfgbytes[2] = { first_byte, first_byte-8 };
    guint32 result;

    result = query_bitstream_site_bytes(bitstream, site, cfgbytes, 2);
    result = ~result;
    luts[i] = reverse_bits(result);
  }

  return;
}

typedef int property_t;

/*
 * Same kind of interface, for different properties. Merge with above
 * once the config bits are unified.
 */
guint16
query_bistream_config(const bitstream_parsed_t *bitstream,
		      const site_t *site, const guint subsite,
		      const property_t *prop) {
  return 0;
}

static const
guchar bram_bit_to_word[16] = {
  6, 4, 2, 0, 8, 10, 12, 14, 15, 13, 11, 9, 1, 3, 5, 7,
};

static const
guint16 bram_offset_to_mask[16] = {
  0x10, 0x800, 0x20, 0x400, 0x40, 0x200, 0x80, 0x100, 0x8, 0x1000, 0x4, 0x2000, 0x2, 0x4000, 0x1, 0x8000,
};

static const
guchar bram_offset_for_bit[16] = {
  1, 1, 1, 2, 1, 1, 1, 3, 1, 1, 1, 2, 1, 1, 1,
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
  const unsigned bram_width = 4 * sizeof(site_descr_t);
  const unsigned site_offset = 2 + sizeof(site_descr_t) + y * bram_width + (bram_width - 2);

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

const gchar *
query_bitstream_bram_parity(const bitstream_parsed_t *bitstream,
			    const site_t *site) {
  /* Actually this is only bit reordering */
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
  const int *frame_count = chip_struct->frame_count;
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
