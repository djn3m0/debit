/*
 * (C) Copyright 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
 * All rights reserved.
 */

#include <glib.h>
#include <glib/gprintf.h>

#include "bitstream_parser.h"
#include "sites.h"
#include "bitstream.h"
#include "design_v4.h"

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

/*
 * XXX To be removed once the databases conform to the new way of doing things
 */
#define STDWIDTH 10
const type_bits_t type_bits[NR_SITE_TYPE] = {
  [SITE_TYPE_NEUTRAL] = {
    .y_width = STDWIDTH,
  },
  [IOB] = {
    .y_width = STDWIDTH,
  },
  [CLB] = {
    .y_width = STDWIDTH,
  },
  [DSP48] = {
    .y_width = STDWIDTH,
  },
  [GCLKC] = {
    .y_width = STDWIDTH,
  },
  [BRAM] = {
    .y_width = STDWIDTH,
  },
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
			  const int cfgbyte) {
  const chip_struct_t *chip_struct = bitstream->chip_struct;
  const site_type_t site_type = site->type;
  const unsigned x = site->type_coord.x;
  const unsigned y = site->type_coord.y;
  const unsigned ymid = chip_struct->row_count;
  unsigned row = (y >> 4);
  /* a bittest should be sufficient for top, but is hard to compute
     (depends on bitlength of the value, which is costly to compute) */
  const unsigned top = (row > ymid) ? 1 : 0;
  const unsigned row_local = y & 0xF;
  const unsigned row_second_half = (y >> 1) & 0x4;

  const guint frame_x = byte_x(cfgbyte);
  /* Middle word contains SECDED and clk information, so we skip it sometimes */
  const guint frame_y = row_local * 10 + row_second_half + byte_y(cfgbyte);
  const gchar *frame;

  row = top ? row >> 1 : row;

  frame = get_frame(bitstream, site_type, row, top, x, frame_x);
  return frame[frame_y];
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
  const guchar cfgbyte_dat = query_bitstream_site_byte(bitstream, site, cfgbit >> 3);
  const guint  cfgbyte_off = cfgbit & 7;

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

  /* query four luts. Bits are MSB first, but in reverse order */
  for (i=0; i<4; i++) {
    luts[i] = 0;
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
  const unsigned *col_count = chip_struct->col_count;
  /* per site, 80 bits on one column, or 10 bytes */
  return type_col_count(col_count, type) * 10;
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
  gsize i;

  for (i = 0; i < nbytes; i++)
    data[i] = query_bitstream_site_byte(parsed, site, i);

  return 0;
}
