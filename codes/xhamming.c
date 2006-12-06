/*
 * Copyright (C) 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
 *
 * Xlx hamming code implementation, as seen in virtex-4 and virtex-5
 */

#include <glib.h>
#include <stdint.h>
#include "xhamming.h"

#define HCODE_STAIRCASE 320
#define HCODE_GAP_START 640
#define HCODE_GAP_STOP (640 + 12)
#define HCODE_END       1312
#define PARITY_BIT     (1<<11)

/*
 * XXX possibly not endian-safe
 */

static int
get_bit(const unsigned char *data, unsigned bit) {
  const uint32_t *u32_data = (const void *) data;
  unsigned u32_offset = bit >> 5;
  unsigned bit_offset = bit & 0x1F;
  guint32  val = GUINT32_FROM_BE(u32_data[u32_offset]);
  int got = (val >> bit_offset) & 1;
  return got;
}

/*
 * This is for standard frames -- routing mainly.
 *
 * Obviously frames with bits that can change, such as RAM data, DistRAM
 * and register frames, do not follow the same rules. I've not yet
 * understood how these work -- maybe the xilinx docs can help on this.
 */

static unsigned
hamming_compute(const unsigned char *data) {
  unsigned bit, xored, hcode = 0;

  /* iterate dumbly over the bits */
  xored = 704 + PARITY_BIT;
  for (bit = 0; bit < HCODE_STAIRCASE; bit++) {
    if (get_bit(data, bit))
      hcode ^= xored;
    xored++;
  }

  xored |= 32;
  for (bit = HCODE_STAIRCASE; bit < HCODE_GAP_START; bit++) {
    if (get_bit(data, bit))
      hcode ^= xored;
    xored++;
  }

  /* skip the code bits themselves */
  xored += (HCODE_GAP_STOP - HCODE_GAP_START);

  for (bit = HCODE_GAP_STOP; bit < HCODE_END; bit++) {
    if (get_bit(data, bit))
      hcode ^= xored;

    xored++;
  }

  return hcode;
}

/* Fast implementation by table-lookup on bytes. Each lut value contains
   4 bits, 3 bits of data, 1 bit of parity. This should be roughly 8
   times faster. */

static unsigned
extract_frame_code(const char *data) {
  const uint32_t *dat = (void *) data;
  return GUINT32_FROM_BE(dat[20]) & 0xFFF;
}

/* population count */
static char
popcount(uint16_t dat) {
  char count = 0;
  unsigned i;

  for (i = 0; i < 11; i++)
    if ( (dat >> i) & 1 )
      count++;

  return count;
}

/* Check frame */
int
check_hamming_frame(const char *data, const uint32_t far) {
  unsigned check = hamming_compute((const void *)data);
  unsigned hamming = extract_frame_code(data);
  unsigned parity = popcount(check) & 1;

  /* FAR should be used to do an intelligent check of the hamming bits
     for variable frames */
  (void) far;

  check ^= (parity << 11);

  if (check != hamming) {
    g_warning("Hamming failed (expected %04x, got %04x)", hamming, check);
    return -1;
  }

  return 0;
}

