/*
 * (C) Copyright 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
 * All rights reserved.
 */

/*
 * Converting a flat DB file to a complex variable-length
 * structure. This method is inspired by the triple-include trick from
 * the DSO howto by Ulrich Drepper.
 */

/*
 * Describing the fundamental structures...
 */

/*
 * The input PIPDB file is formatted as thus
 * _PIP_WHOLE_ENTRY(
 * ndata,
 * _PIP_CTRL_ENTRY(endwire, ncfgbits, cfgbits...),
 * _PIP_CTRL_ENTRY(wire, cfgbitmask), ...
 * )
 * ...
 *
 * the file wires.h must be included, so that we get a proper
 * translation of wire names to wire indexes.
 */

#include <stddef.h>
#include <stdint.h>

#include "virtex2/wires.h"

typedef uint16_t wire_atom_t;

typedef struct _pip_control_t {
  wire_atom_t endwire;
  unsigned char size;
  uint32_t data[];
} pip_control_t;

typedef struct _pip_data_t {
  wire_atom_t startwire;
  uint32_t cfgdata;
} pip_data_t;

typedef struct _pip_data_array_t {
  unsigned char size;
  pip_data_t data[];
} pip_data_array_t;

/* This is the string array & string lookup table generation */

#define PIPDB "demo.db"

#define CTRLFIELD(line) FIELD1(line)
#define DATAFIELD(line) FIELD2(line)
#define FIELD1(line) ctrl##line
#define FIELD2(line) data##line

#define CTRL_STRUCT(name, len) struct { wire_atom_t endwire;  unsigned char size;  uint32_t data[len]; } name
#define CTRL_STRUCT_FILL(vendwire, vsize_ctrl, vsize_data, list...) { .endwire = vendwire, .size = vsize_ctrl, .data = { list } }

#define DATA_STRUCT(name, len) struct { unsigned char size;  pip_data_t data[len]; } name
#define DATA_STRUCT_FILL(vstartwire, vcfgdata) { .startwire = vstartwire, .cfgdata = vcfgdata }

static const struct pips_t {
#define _PIP_WHOLE_ENTRY(ctrl, k, data) CTRL_STRUCT(CTRLFIELD(__LINE__), ctrl); DATA_STRUCT(DATAFIELD(__LINE__), k);
#define _PIP_CTRL_ENTRY(s, n, ctrllist) n
#define _PIP_CTRL_LIST(...)
#define _PIP_DATA_LIST(...)
#define _PIP_DATA_ENTRY(wire, cfgbit)

#include PIPDB

#undef _PIP_CTRL_ENTRY
#undef _PIP_DATA_ENTRY
#undef _PIP_WHOLE_ENTRY
#undef _PIP_CTRL_LIST
#undef _PIP_DATA_LIST

} pipdb = {
#define _PIP_WHOLE_ENTRY(ctrl, vsize, vdata) ctrl, { .size = vsize, .data = { vdata } },
#define _PIP_CTRL_ENTRY(s, n, ctrllist...) CTRL_STRUCT_FILL(s, n, k, ctrllist)
#define _PIP_CTRL_LIST(...) __VA_ARGS__
#define _PIP_DATA_LIST(...) __VA_ARGS__
#define _PIP_DATA_ENTRY(s, n) DATA_STRUCT_FILL(s, n)

#include PIPDB

#undef _PIP_CTRL_ENTRY
#undef _PIP_DATA_ENTRY
#undef _PIP_WHOLE_ENTRY
#undef _PIP_CTRL_LIST
#undef _PIP_DATA_LIST

};

/* Then the iterators */
