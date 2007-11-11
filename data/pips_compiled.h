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
#define INSTANCENAME(a, x) a ## x
#define CTRLFIELD(line) FIELD1(line)
#define DATAFIELD(line) FIELD2(line)
#define FIELD1(line) ctrl##line
#define FIELD2(line) data##line

#define PACK __attribute__((packed))

/* First structures, the arrays
   for the variable-length data */

/* First structure: control data */

#define CONTROLSTRUCTNAME(x) STRUCTNAME(pips_control_, x)
#define CONTROLSTRUCTINSTANCE(x) INSTANCENAME(ctrl_, x)
#define CTRL_STRUCT(name, len) CONTROLSTRUCTTYPE name[len]
#define CTRL_STRUCT_FILL(list...) { list }

/* TODO: change this structure to uint16_t elements.
   This will halve the structure's size */

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
  CONTROLSTRUCTTYPE control_tab[0];
} PACK CONTROLSTRUCTINSTANCE(DBNAME) = { {
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

#undef CTRL_STRUCT
#undef CTRL_STRUCT_FILL

/* Second structure: pip actual data */

#define DATASTRUCTNAME(x) STRUCTNAME(pips_data_, x)
#define DATASTRUCTINSTANCE(x) INSTANCENAME(data_, x)
#define DATA_STRUCT(name, len) DATASTRUCTTYPE name[len]
#define DATA_STRUCT_FILL(vstartwire, vcfgdata) { .startwire = DBWIRE_##vstartwire, .cfgdata = vcfgdata }

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
  DATASTRUCTTYPE data_tab[0];
} PACK DATASTRUCTINSTANCE(DBNAME) = { {
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

#undef DATA_STRUCT
#undef DATA_STRUCT_FILL

/* Last but not least: simple array with references */
#define ARRAYINSTANCE(x) INSTANCENAME(pipdb_, x)

static const pip_control_t ARRAYINSTANCE(DBNAME)[] = {
#define _PIP_CTRL_ENTRY(s, n, ctrllist...) DBWIRE_##s
#define _PIP_CTRL_LIST(...)
#define _PIP_DATA_LIST(...)
#define _PIP_DATA_ENTRY(s, n)
#define _PIP_WHOLE_ENTRY(vendwire, vsize, vdata) \
{ .endwire = vendwire, \
  .ctrloffset = offsetof(union CONTROLSTRUCTNAME(DBNAME), CTRLFIELD(__LINE__)) / sizeof(CONTROLSTRUCTTYPE), \
  .ctrlsize = N_ELEMS(CONTROLSTRUCTINSTANCE(DBNAME).CTRLFIELD(__LINE__)), \
  .dataoffset = offsetof(union DATASTRUCTNAME(DBNAME), CTRLFIELD(__LINE__)) / sizeof(DATASTRUCTTYPE), \
  .datasize = N_ELEMS(DATASTRUCTINSTANCE(DBNAME).CTRLFIELD(__LINE__)) \
},

#include PIPDB

#undef ARRAYINSTANCE

#undef _PIP_CTRL_ENTRY
#undef _PIP_DATA_ENTRY
#undef _PIP_WHOLE_ENTRY
#undef _PIP_CTRL_LIST
#undef _PIP_DATA_LIST

};

#undef CONTROLSTRUCTNAME
#undef CONTROLSTRUCTINSTANCE

#undef DATASTRUCTNAME
#undef DATASTRUCTINSTANCE

#undef STRUCTNAME
#undef INSTANCENAME
#undef CTRLFIELD
#undef DATAFIELD
#undef FIELD1
#undef FIELD2

#undef PACK
