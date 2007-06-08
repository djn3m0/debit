/*
 * (C) Copyright 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
 * All rights reserved.
 */

#ifndef _HAS_XDLOUT_H
#define _HAS_XDLOUT_H

#include "connexity.h"
#include "localpips.h"
#include "sites.h"

void print_design(parsed_header_t *header);

void print_nets(nets_t *net,
		const pip_db_t *pipdb,
		const chip_descr_t *cdb);

void
print_slices(const pip_parsed_dense_t *pipdat,
	     const pip_db_t *pipdb,
	     const chip_descr_t *chip);

#endif /* _HAS_XDLOUT_H */
