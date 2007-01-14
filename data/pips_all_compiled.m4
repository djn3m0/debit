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
`#'define PIPDB "$2/all.db"
`#'define SITE_WIDTH $3
`#'include "data/pips_compiled.h"
`#'undef SITE_WIDTH
`#'undef PIPDB
`#'undef DBNAME
)dnl
include(PIPSDB)dnl
undefine(`_PIPS_DB')dnl

static const pipdb_control_t dbrefs[NR_SITE_TYPE] = {
define(_PIPS_DB,[$1] = { .pipctrl = pipdb_$1`,' .pipctrl_len = N_ELEMS(pipdb_$1)`,' .pipctrldata = ctrl_$1.control_tab`,' .pipdatadata = data_$1.data_tab }`,')dnl
include(PIPSDB)dnl
undefine(_PIPS_DB)dnl
};

