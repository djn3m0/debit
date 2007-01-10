/*
 * (C) Copyright 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
 * All rights reserved.
 */

#ifndef _HAS_PIPS_COMPILED_COMMON_H
#define _HAS_PIPS_COMPILED_COMMON_H

#include <stddef.h>
#include <stdint.h>

#define N_ELEMS(a) (unsigned)(sizeof(a) / sizeof(a[0]))

typedef uint16_t wire_atom_t;

/* This is a simple array */
typedef struct _pip_control_t {
  wire_atom_t endwire;
  unsigned ctrloffset;
  unsigned char ctrlsize;
  unsigned dataoffset;
  unsigned char datasize;
} pip_control_t;

/* This will fit into on variable-length array */
typedef uint32_t ctrldata;

/* This will fit in two variable-length arrays */
typedef struct _pip_data_t {
  wire_atom_t startwire;
  uint32_t cfgdata;
} pip_data_t;

#endif /* _HAS_PIPS_COMPILED_COMMON_H */
