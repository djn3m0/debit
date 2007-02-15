/*
 * (C) Copyright 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
 * All rights reserved.
 */

#ifndef _HAS_CFGBIT_H
#define _HAS_CFGBIT_H

/****
 * Cfgbit configuration:
 * - lower 8 bits are the position of the *bit* on the Y axis. Thus, to
 * get byte positionning, one should ask for the (bitposition >> 3)
 * - next 8 bits are the position of the bit on the X axis, identical
 * to the position of the byte on the X axis.
 */

#define CFGBIT_Y_LENGTH 8
#define CFGBIT_Y_MASK  ((1 << CFGBIT_Y_LENGTH) - 1)
#define CFGBIT_BYTE_OFFSET_LENGTH 3
#define CFGBIT_BYTE_OFFSET_MASK  ((1 << CFGBIT_BYTE_OFFSET_LENGTH) - 1)
#define CFGBIT_Y_BYTE_LEN  (CFGBIT_Y_LENGTH - CFGBIT_BYTE_OFFSET_LENGTH)
#define CFGBIT_Y_BYTE_MASK ((1 << CFGBIT_Y_BYTE_LEN) - 1)


static inline
unsigned byte_y(const unsigned cfgbit) {
  return (cfgbit >> CFGBIT_BYTE_OFFSET_LENGTH) & CFGBIT_Y_BYTE_MASK;
}

static inline
unsigned bit_y(const unsigned cfgbit) {
  return (cfgbit & CFGBIT_Y_MASK);
}

static inline
unsigned bit_offset(const unsigned cfgbit) {
  return (cfgbit & CFGBIT_BYTE_OFFSET_MASK);
}

static inline
unsigned bit_x(const unsigned cfgbit) {
  return (cfgbit >> CFGBIT_Y_LENGTH);
}

static inline
unsigned byte_x(const unsigned cfgbit) {
  return (cfgbit >> CFGBIT_Y_LENGTH);
}

static inline
unsigned byte_addr(const unsigned cfgbit) {
  return (cfgbit | CFGBIT_BYTE_OFFSET_MASK);
}

static inline
unsigned assemble_cfgbit(const unsigned x,
			 const unsigned y) {
  return (y | (x << CFGBIT_Y_LENGTH));
}

/* Temporary conversion function for the database. To get rid of once
   the database is moved to the new format */
#if defined(VIRTEX5)

#define BITPOS_INVERT(val, width) ((val) << 3)
static inline
unsigned bitpos_invert(const unsigned bitpos, const unsigned width) {
  (void) width;
  return bitpos << 3;
}

#else /* This is still in use for virtex4 and virtex2 */

#define BITPOS_INVERT(val, width) (((width - 1) - (val)) << 3)
static inline
unsigned bitpos_invert(const unsigned bitpos, const unsigned width) {
  return ((width - 1) - bitpos) << 3;
}

#endif

static inline
unsigned bitpos_to_cfgbit(const unsigned bitpos, const unsigned width) {
  const unsigned xoff = bitpos / (8*width);
  const unsigned yoff = bitpos % (8*width);
  const unsigned nyoff = bitpos_invert(yoff>>3, width) | (yoff & 7);
  return assemble_cfgbit(xoff,nyoff);
}

/* Unrolled macro version of the above */
#define XOFF(val, width) ((val) / (8*width))
#define YOFF(val, width) ((val) % (8*width))
#define NYOFF(val, width) (BITPOS_INVERT(YOFF(val, width) >> 3, width) | (YOFF(val, width) & 7))
#define CFG_ASSEMBLE(x, y) ((y) | (x) << CFGBIT_Y_LENGTH)
#define CFGBIT(bitpos, width) CFG_ASSEMBLE(XOFF(bitpos, width), NYOFF(bitpos, width))

typedef struct _type_bits {
  /** Column type */
  guint col_type;
  /** Frames to skip at the beginning of the type array */
  guint x_type_off;
  /** Bytes to skip at the beginning of the type array */
  gint y_offset;
  guint y_width;
  guint row_count;
} type_bits_t;

const type_bits_t type_bits[NR_SITE_TYPE];

#endif /* _HAS_CFGBIT_H */
