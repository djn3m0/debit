/*
 * (C) Copyright 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
 * All rights reserved.
 */

/*
 * We need to consolidate multiple pips database. This file does it.
 */

include(WIREDB)dnl
include(SITEDB)dnl

define(_PIPS_DB,
`#'define DBNAME $1
`#'define DBNAME1 controldb_$1
`#'define DBNAME2 datadb_$1
`#'define DBNAME3 pipdb_$1
`#'define PIPDB "$2/all.db"
`#'include "data/pips_compiled.h"
`#'undef DBNAME1
`#'undef DBNAME2
`#'undef DBNAME3
`#'undef DBNAME
`#'undef PIPDB
)dnl
include(PIPSDB)dnl
undefine(`_PIPS_DB')dnl

static const pip_control_t *dbrefs[] = {
define(_PIPS_DB,[$1] = pipdb_$1`,')dnl
include(PIPSDB)dnl
undefine(_PIPS_DB)dnl
};

