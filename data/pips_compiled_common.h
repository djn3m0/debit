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

#ifndef _HAS_PIPS_COMPILED_COMMON_H
#define _HAS_PIPS_COMPILED_COMMON_H

#include <stddef.h>
#include <stdint.h>

#define _CTRL_ENTRY(a) CFGBIT(a, SITE_WIDTH)
#define N_ELEMS(a) (unsigned)(sizeof(a) / sizeof(a[0]))

#define CONTROLSTRUCTTYPE uint32_t
#define DATASTRUCTTYPE pip_data_t

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

typedef struct _pipdb_control_t {
  const pip_control_t *pipctrl;
  unsigned pipctrl_len;
  const uint32_t *pipctrldata;
  const pip_data_t *pipdatadata;
} pipdb_control_t;

/* Utility macros */

/* #define STRUCTNAME(a, x) a ## x ## _t */
/* #define INSTANCENAME(a, x) a ## x */
/* #define CTRLFIELD(line) FIELD1(line) */
/* #define DATAFIELD(line) FIELD2(line) */
/* #define FIELD1(line) ctrl##line */
/* #define FIELD2(line) data##line */

/* #define PACK __attribute__((packed)) */

/* /\* First structure: control data *\/ */

/* #define CONTROLSTRUCTNAME(x) STRUCTNAME(pips_control_, x) */
/* #define CONTROLSTRUCTINSTANCE(x) INSTANCENAME(ctrl_, x) */
/* #define CTRL_STRUCT(name, len) uint32_t name[len] */
/* #define CTRL_STRUCT_FILL(list...) { list } */

/* /\* Second structure: pip actual data *\/ */

/* #define DATASTRUCTNAME(x) STRUCTNAME(pips_data_, x) */
/* #define DATASTRUCTINSTANCE(x) INSTANCENAME(data_, x) */
/* #define DATA_STRUCT(name, len) pip_data_t name[len] */
/* #define DATA_STRUCT_FILL(vstartwire, vcfgdata) { .startwire = vstartwire, .cfgdata = vcfgdata } */

/* /\* Last but not least: simple array with references *\/ */
/* #define ARRAYINSTANCE(x) INSTANCENAME(array_, x) */

#endif /* _HAS_PIPS_COMPILED_COMMON_H */
