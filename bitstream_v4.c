/*
 * (C) Copyright 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
 * All rights reserved.
 */

#include <glib.h>
#include <glib/gprintf.h>

#include "bitstream_parser.h"
#include "sites.h"
#include "bitstream.h"

#include "design.h"
#include "cfgbit.h"

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

#if defined(VIRTEX4)

#define STDWIDTH 10

const type_bits_t type_bits[NR_SITE_TYPE] = {
  [SITE_TYPE_NEUTRAL] = {
    .y_width = STDWIDTH,
  },
  [IOB] = {
    .col_type = V4C_IOB,
    .y_width = STDWIDTH,
  },
  [CLB] = {
    .col_type = V4C_CLB,
    .y_width = STDWIDTH,
  },
  [DSP48] = {
    .col_type = V4C_DSP48,
    .y_width = STDWIDTH,
  },
  [GCLKC] = {
    .col_type = V4C_GCLK,
    .y_width = STDWIDTH,
  },
  [BRAM] = {
    .col_type = V4C_BRAM_INT,
    .y_width = STDWIDTH,
  },
};
#elif defined(VIRTEX5)

#define STDWIDTH 8

const type_bits_t type_bits[NR_SITE_TYPE] = {
  [SITE_TYPE_NEUTRAL] = {
    .y_width = STDWIDTH,
  },
  [IOB] = {
    .col_type = V5C_IOB,
    .y_width = STDWIDTH,
  },
  [CLB] = {
    .col_type = V5C_CLB,
    .y_width = STDWIDTH,
  },
  [DSP48] = {
    .col_type = V5C_DSP48,
    .y_width = STDWIDTH,
  },
  [GCLKC] = {
    .col_type = V5C_GCLK,
    .y_width = STDWIDTH,
  },
  [BRAM] = {
    .col_type = V5C_BRAM_INT,
    .y_width = STDWIDTH,
  },
};
#endif

/*
 * Untyped query functions
 */

#if defined (VIRTEX4)

/*
 * Helper function
 */
static unsigned char
mirror_byte(const unsigned char dat) {
  unsigned char res = 0;
  int i;
  for (i = 0; i < 8; i++)
    if (dat & (1 << i))
      res |= (1 << (7-i));
  return res;
}
/* static unsigned char */
/* mirror_byte(const unsigned char dat) { */
/*   return ((dat * 0x0802LU & 0x22110LU) | (dat * 0x8020LU & 0x88440LU)) * 0x10101LU >> 16; */
/* } */

/* static unsigned char */
/* mirror_byte(const unsigned char dat) { */
/*   unsigned result = (dat >> 4) | (dat << 4); */
/*   result = ((result & 0xcc) >> 2 ) | ((result & 0x33) << 2); */
/*   result = ((result & 0xaa) >> 1 ) | ((result & 0x55) << 1); */
/*   return result; */
/* } */

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
			  const int cfgbyte) {
  const chip_struct_t *chip_struct = bitstream->chip_struct;
  const site_type_t site_type = site->type;
  const unsigned x = site->type_coord.x;
  const unsigned y = site->type_coord.y;
  const unsigned ymid = chip_struct->row_count;
  /* Rows are packed by groups of 16 */
  unsigned row = (y >> 4);
  /* a bittest should be sufficient for top, but is hard to compute
     (depends on bitlength of the value, which is costly to compute) */
  const unsigned top = (row >= ymid) ? 0 : 1;
  const unsigned row_local = y & 0xF;
  /* We must skip 4 bytes of SECDED and CLK information in the middle of
   * the frame */
  const unsigned row_second_half = (y >> 1) & 0x4;
  const guint frame_x = byte_x(cfgbyte);
  /* Middle word contains SECDED and clk information, so we skip it sometimes */
  guint frame_y = row_local * STDWIDTH + row_second_half + byte_y(cfgbyte);
  const gchar *frame;
  unsigned char byte;

  /* When top is one, the row numbering is inverted, and bits are mirrored */
  row = top ? (ymid - 1 - row) : row - ymid;
  frame = get_frame(bitstream, type_bits[site_type].col_type, row, top, x, frame_x);
  frame_y = top ? (164 - 1 - frame_y) : frame_y;

  /* The adressing here is a bit strange, due to the frame byte order */
  byte = frame[frame_y ^ 0x3];
  return top ? byte : mirror_byte(byte);
}

#elif defined(VIRTEX5)

static guchar
query_bitstream_site_byte(const bitstream_parsed_t *bitstream,
			  const csite_descr_t *site,
			  const int cfgbyte) {
  const chip_struct_t *chip_struct = bitstream->chip_struct;
  const site_type_t site_type = site->type;
  const unsigned x = site->type_coord.x;
  const unsigned y = site->type_coord.y;
  const unsigned ymid = chip_struct->row_count;
  /* Rows are packed by groups of 20 */
  unsigned row = (y / 20);
  /* a bittest should be sufficient for top, but is hard to compute
     (depends on bitlength of the value, which is costly to compute) */
  const unsigned top = (row >= ymid) ? 0 : 1;
  const unsigned row_local = y % 20;
  const unsigned row_second_half = (row_local >= 10) ? 4 : 0;

  const guint frame_x = byte_x(cfgbyte);
  /* Middle word contains SECDED and clk information, so we skip it sometimes */
  guint frame_y = row_local * STDWIDTH + row_second_half + byte_y(cfgbyte);
  const gchar *frame;

  /* When top is one, the row numbering is inverted */
  row = top ? (ymid - 1 - row) : row - ymid;
  frame = get_frame(bitstream, type_bits[site_type].col_type, row, top, x, frame_x);

  /* The adressing here is a bit strange, due to the frame byte order */
  return frame[frame_y ^ 0x3];
}

#endif

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

__attribute__((unused)) static guint32
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
  const guchar cfgbyte_dat = query_bitstream_site_byte(bitstream, site, byte_addr(cfgbit));
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
  guint32 result = 0;
  gsize i;

  for(i = 0; i < nbits; i++)
    /* XXX move this to non-conditional */
    if (query_bitstream_site_bit(bitstream, site, cfgbits[i]))
      result |= 1 << i;

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

  (void) bitstream;
  (void) site;

  /* query eight luts. Bits are MSB first, but in reverse order */
  for (i=0; i<8; i++) {
    luts[i] = 0;
  }

  return;
}

typedef int property_t;

/*
 * Same kind of interface, for different properties. Merge with above
 * once the config bits are unified.
 */
__attribute__((unused)) static guint16
query_bistream_config(const bitstream_parsed_t *bitstream,
		      const site_t *site, const guint subsite,
		      const property_t *prop) {
  (void) bitstream;
  (void) site;
  (void) subsite;
  (void) prop;
  return 0;
}

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
  guint16 *bram_data = g_new0(guint16,64*16);
  (void) bitstream;
  (void) site;
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
  /* Get the col type from the site type */
  const design_col_t col_type = type_bits[type].col_type;
  /* per site, 80 bits in each frame -- 10 bytes */
  return type_frame_count(chip_struct, col_type) * STDWIDTH;
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
    g_print("From site, getting pos %i ", pos);
    data[i] = query_bitstream_site_byte(parsed, site, pos);
  }

  return 0;
}
