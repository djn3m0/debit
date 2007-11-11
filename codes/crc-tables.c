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
 * Table lookup dump for the standard CRC algorithms
 */

#include <stdio.h>
#include <stdint.h>

#include "crc32-c.h"

#define POLY_CRC16_IBM 0x8005
#define POLY_CRC16_IBM_REFLEX 0xa001

static uint32_t g[8];
/* Compute the basis polynomials */

static void
crc16_compute_genpolys(unsigned nbits, unsigned poly) {
  unsigned j;
  g[0] = poly;
  for (j = 1; j < nbits; j++) {
    g[j] = g[j-1] >> 1;
    if (g[j-1] & 1)
      g[j] ^= g[0];
    g[j] &= 0xffff;
  }
}

/* dumps a table for CRC16 lut computation */
static void
crc16_dump_table(unsigned nbits, unsigned poly, FILE *out) {
  unsigned nelems = 1 << nbits;
  unsigned i, j;

  crc16_compute_genpolys(nbits, poly);

  for (i = 0; i < nelems; i++) {
    uint16_t f = 0;
    uint16_t T = i;
    for (j = 0; j < nbits; j++) {
      /* LSB first here, so all OK */
      if (T & 1)
	f ^= g[nbits - 1 -j];
      T >>= 1;
    }
    fprintf(out, " 0x%04x,", f);
    if (i % 8 == 7)
      fprintf(out, "\n");
  }
}

static void
crc32_compute_genpolys(unsigned nbits, uint32_t poly) {
  unsigned j;
  g[0] = poly;
  for (j = 1; j < nbits; j++) {
    g[j] = g[j-1] >> 1;
    if (g[j-1] & 1)
      g[j] ^= g[0];
  }
}

/* dumps a table for CRC16 lut computation */
static void
crc32_dump_table(unsigned nbits, uint32_t poly, FILE *out) {
  unsigned nelems = 1 << nbits;
  unsigned i, j;

  crc32_compute_genpolys(nbits, poly);

  for (i = 0; i < nelems; i++) {
    uint32_t f = 0;
    uint32_t T = i;
    for (j = 0; j < nbits; j++) {
      /* LSB first here, so all OK */
      if (T & 1)
	f ^= g[nbits - 1 -j];
      T >>= 1;
    }
    fprintf(out, " 0x%08xL,", f);
    if (i % 8 == 7)
      fprintf(out, "\n");
  }
}

int main(int argc, char *argv[], char **env) {
  FILE *out = stdout;
  fprintf(out, "crc_table_ibm[256] = {\n");
  crc16_dump_table(8, POLY_CRC16_IBM_REFLEX, out);
  fprintf(out, "};\n");

  fprintf(out, "crc_table_ibm[32] = {\n");
  crc16_dump_table(5, POLY_CRC16_IBM_REFLEX, out);
  fprintf(out, "};\n");

  fprintf(out, "crc32c_table[256] = {\n");
  crc32_dump_table(8, CRC32C_POLY_REFLEX, out);
  fprintf(out, "};\n");

  fprintf(out, "crc32c_table_addr5[32] = {\n");
  crc32_dump_table(5, CRC32C_POLY_REFLEX, out);
  fprintf(out, "};\n");

  return 0;
}
