/*
 * (C) Copyright 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
 * All rights reserved.
 */

#ifndef _HAS_CONNEXITY_H
#define _HAS_CONNEXITY_H

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

void print_nets(nets_t *net,
		const pip_db_t *pipdb,
		const chip_descr_t *cdb);

#endif /* _HAS_CONNEXITY_H */
