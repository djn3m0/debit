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

/*
 * Fast CRC-32 Castagnoli implementation
 */

#ifndef _HAS_CRC32_C
#define _HAS_CRC32_C

#include <stdint.h>
#include <sys/types.h>

#define CRC32C_POLY        0x1edc6f41
#define CRC32C_POLY_REFLEX 0x82f63b78

extern const uint32_t crc32c_table[256];
extern const uint32_t crc32c_table_addr5[32];

extern uint32_t crc32c(uint32_t crc, const uint8_t *buffer, size_t len);

/* Uses the lowest bit of the bit argument */
static inline uint32_t
crc32c_shift(unsigned bit, uint32_t bcc) {
  unsigned val = ((bcc >> 31) ^ bit) & 1;
  if (val != 0) {
    bcc <<= 1;
    bcc ^= CRC32C_POLY;
  } else {
    bcc <<= 1;
  }

  return bcc;
}

static inline uint32_t
crc32c_shift_reflex(unsigned bit, uint32_t bcc) {
  unsigned val = (bcc ^ bit) & 1;
  if (val != 0) {
    bcc >>= 1;
    bcc ^= CRC32C_POLY_REFLEX;
  } else {
    bcc >>= 1;
  }

  return bcc;
}

static inline uint32_t
crc32c_byte(uint32_t crc, const uint8_t c)
{
  const unsigned char lut = (crc ^ c) & 0xFF;
  return (crc >> 8) ^ crc32c_table[lut];
}

/** crc_ibm_addr5 - recomputes the new CRC value by shifting the lower 5
 *  bits of val in the register.
 */
static inline uint32_t
crc32c_addr5(uint32_t crc, const uint8_t c) {
  const unsigned char lut = (crc ^ c) & 0x1F;
  return (crc >> 5) ^ crc32c_table_addr5[lut];
}

#endif /* _HAS_CRC32_C */
