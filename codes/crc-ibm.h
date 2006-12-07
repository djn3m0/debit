/*
 * Copyright (C) 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
 *
 * Fast implementation IBM CRC implementation, inspired by the kernel
 * CRC-CCITT implementation and litterature.
 *
 * This computation could be accelerated even more, but at the cost of a
 * different strange computation at the beginning of the computation,
 * which is very hard to fit in the parser.
 */

#ifndef _CRC_IBM_H
#define _CRC_IBM_H

#include <stdint.h>
#include <sys/types.h>

extern const uint16_t crc_ibm_table[256];
extern const uint16_t crc_ibm_table_addr5[32];

extern uint16_t crc_ibm(uint16_t crc, const uint8_t *buffer, size_t len);

/*
 * Shift-register implementation of the IBM CRC.
 */

static inline uint16_t
crc_ibm_shift(unsigned bit, uint16_t bcc) {
  unsigned val = ((bcc >> 15) ^ bit) & 1;
  if (val != 0) {
    bcc <<= 1;
    bcc ^= 0x8005;
  } else {
    bcc <<= 1;
  }

  return bcc & 0xffff;
}

/*
 * Shift-register implementation, with bits in the reverse order.
 */

static inline uint16_t
crc_ibm_shift_reflex(unsigned bit, uint16_t bcc) {
  unsigned val = (bcc ^ bit) & 1;
  if (val != 0) {
    bcc >>= 1;
    bcc ^= 0xa001;/* reflex of 0x8005 */
  } else {
    bcc >>= 1;
  }

  return bcc & 0xffff;
}

static inline uint16_t
crc_ibm_byte(uint16_t crc, const uint8_t c)
{
  const unsigned char lut = (crc ^ c) & 0xFF;
  return (crc >> 8) ^ crc_ibm_table[lut];
}

/** crc_ibm_addr5 - recomputes the new CRC value by shifting the lower 5
 *  bits of val in the register.
 */
static inline uint16_t
crc_ibm_addr5(uint16_t crc, const uint8_t c) {
  const unsigned char lut = (crc ^ c) & 0x1F;
  return (crc >> 5) ^ crc_ibm_table_addr5[lut];
}

/*
 * Addr4 may also be needed e.g. for Spartan
 */

#endif /* _CRC_IBM_H */
