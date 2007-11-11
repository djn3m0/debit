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
