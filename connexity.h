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

#ifndef _HAS_CONNEXITY_H
#define _HAS_CONNEXITY_H

#include <glib.h>
#include "localpips.h"
#include "sites.h"

/** \brief Structure describing all nets in an FPGA
 *
 * This structure is an N-ary tree. The first-level nodes are the
 * different nets; then from then on each GNode contains a pointer to
 * some (pip + site) reference, and links between GNodes reflect copper
 * wiring inside the chip (ie, wiring which is not part of the explicit
 * description that we get from the bitstream, but which is contained in
 * the physical chip layout).
 *
 */

typedef struct _nets_t {
  GNode *head;
} nets_t;

nets_t *build_nets(const pip_db_t *pipdb,
		   const chip_descr_t *cdb,
		   const pip_parsed_dense_t *pipdat);

void free_nets(nets_t *);

#endif /* _HAS_CONNEXITY_H */
