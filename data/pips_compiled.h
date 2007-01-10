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
 * The input PIPDB file is formatted as thus
 *_PIP_WHOLE_ENTRY(
 * _PIP_CTRL_ENTRY(S6BEG4,7,_PIP_CTRL_LIST(1473,1475,1553,1555,1635,1712,1714)),
 * 12,
 * _PIP_DATA_LIST(
 * _PIP_DATA_ENTRY(OMUX_WS1,18),
 * ...)
 * )
 * )
 */

#include "pips_compiled_common.h"

/* This is the string array & string lookup table generation */

#define STRUCTNAME(a, x) a ## x ## _t
#define CTRLFIELD(line) FIELD1(line)
#define DATAFIELD(line) FIELD2(line)
#define FIELD1(line) ctrl##line
#define FIELD2(line) data##line

#define PACK __attribute__((packed))

/* First structures, the arrays
   for the variable-length data */

/* First structure: control data */

#define CONTROLSTRUCTNAME(x) STRUCTNAME(pips_control_, x)
#define CTRL_STRUCT(name, len) uint32_t name[len]
#define CTRL_STRUCT_FILL(list...) { list }

static const union CONTROLSTRUCTNAME(DBNAME) {
  struct {
#define _PIP_CTRL_ENTRY(s, n, ctrllist) n
#define _PIP_CTRL_LIST(...)
#define _PIP_DATA_LIST(...)
#define _PIP_DATA_ENTRY(wire, cfgbit)

#define _PIP_WHOLE_ENTRY(ctrl, k, data) CTRL_STRUCT(CTRLFIELD(__LINE__), ctrl);

#include PIPDB

#undef _PIP_CTRL_ENTRY
#undef _PIP_DATA_ENTRY
#undef _PIP_WHOLE_ENTRY
#undef _PIP_CTRL_LIST
#undef _PIP_DATA_LIST
  };
  uint32_t control_tab[0];
} PACK DBNAME1 = { {
#define _PIP_CTRL_ENTRY(s, n, ctrllist...) CTRL_STRUCT_FILL(ctrllist)
#define _PIP_CTRL_LIST(...) __VA_ARGS__
#define _PIP_DATA_LIST(...)
#define _PIP_DATA_ENTRY(s, n)

#define _PIP_WHOLE_ENTRY(ctrl, vsize, vdata) ctrl,

#include PIPDB

#undef _PIP_CTRL_ENTRY
#undef _PIP_DATA_ENTRY
#undef _PIP_WHOLE_ENTRY
#undef _PIP_CTRL_LIST
#undef _PIP_DATA_LIST

} };

/* Second structure: pip actual data */

#define DATASTRUCTNAME(x) STRUCTNAME(pips_data_, x)
#define DATA_STRUCT(name, len) pip_data_t name[len]
#define DATA_STRUCT_FILL(vstartwire, vcfgdata) { .startwire = vstartwire, .cfgdata = vcfgdata }

static const union DATASTRUCTNAME(DBNAME) {
  struct {
#define _PIP_CTRL_ENTRY(s, n, ctrllist...)
#define _PIP_CTRL_LIST(...)
#define _PIP_DATA_LIST(...) __VA_ARGS__
#define _PIP_DATA_ENTRY(wire, cfgbit)
#define _PIP_WHOLE_ENTRY(ctrl, k, data) DATA_STRUCT(CTRLFIELD(__LINE__), k);

#include PIPDB

#undef _PIP_CTRL_ENTRY
#undef _PIP_DATA_ENTRY
#undef _PIP_WHOLE_ENTRY
#undef _PIP_CTRL_LIST
#undef _PIP_DATA_LIST
  };
  pip_data_t data_tab[0];
} PACK DBNAME2 = { {
#define _PIP_CTRL_ENTRY(s, n, ctrllist...)
#define _PIP_CTRL_LIST(...)
#define _PIP_DATA_LIST(...) __VA_ARGS__
#define _PIP_DATA_ENTRY(s, n) DATA_STRUCT_FILL(s, n)
#define _PIP_WHOLE_ENTRY(ctrl, vsize, vdata) { vdata },

#include PIPDB

#undef _PIP_CTRL_ENTRY
#undef _PIP_DATA_ENTRY
#undef _PIP_WHOLE_ENTRY
#undef _PIP_CTRL_LIST
#undef _PIP_DATA_LIST
} };

/* Last but not least: simple array with references */

static const pip_control_t DBNAME3[] = {
#define _PIP_CTRL_ENTRY(s, n, ctrllist...) s
#define _PIP_CTRL_LIST(...)
#define _PIP_DATA_LIST(...)
#define _PIP_DATA_ENTRY(s, n)
#define _PIP_WHOLE_ENTRY(vendwire, vsize, vdata) \
{ .endwire = vendwire, \
  .ctrloffset = offsetof(union CONTROLSTRUCTNAME(DBNAME), CTRLFIELD(__LINE__)), \
  .ctrlsize = N_ELEMS(DBNAME1.CTRLFIELD(__LINE__)), \
  .dataoffset = offsetof(union DATASTRUCTNAME(DBNAME), CTRLFIELD(__LINE__)), \
  .datasize = N_ELEMS(DBNAME2.CTRLFIELD(__LINE__)) \
},

#include PIPDB

#undef _PIP_CTRL_ENTRY
#undef _PIP_DATA_ENTRY
#undef _PIP_WHOLE_ENTRY
#undef _PIP_CTRL_LIST
#undef _PIP_DATA_LIST

};

/* Then the iterators... */
