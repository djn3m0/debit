/*
 * (C) Copyright 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
 * All rights reserved.
 */

#include <glib.h>
#include "wiring.h"

/*
 * Connexity analysis
 */

typedef struct _sited_wire {
  struct wire_details_t *wire;
  struct site_details_t *site;
} sited_wire_t;

/** \brief Structure describing all nets in an FPGA
 *
 * This structure is an N-ary tree. The first-level nodes are the
 * different nets; then from then on
 *
 * The data in the nodes are sited wires.
 */

typedef struct _nets_t {
  GNode *head;
} nets_t;

/**
 *
 *
 *
 */

/*
 * Try to reach all pips from an endpoint and site,
 * and to reconstruct a 'net' from there, for as long as we can.
 *
 * @param details_matrix the debitted matrix containing the actual pips in
 * the bitstream in cached form
 * @param cdb the chip database containing global pips (the chip copper layout)
 * @param site the starting site
 * @param wire the starting wire in the site
 *
 * @returns the start of the inserted net
 */
GNode *build_net_from(const site_details_t *details_matrix,
		      const chip_db_t *cdb, net_t *nets,
		      const sited_wire_t *wire) {

  do {
    /* First query the localpip database */
    const gchar *start;
    start = get_wire_startpoint(pipdb, bitstream, site, wire->wire->name);

    if (!start) {
      sited_wire_t *startwire;
      int err;

      err = get_interconnect_startpoint(startwire->site, startwire->wire,
					wire->site, wire->wire);

      if (err)
	return add_new_net(wire);
    }

    /* match: is the other endpoint already in a net ? */
    if (is_in_net(start))
      return add_dependency(start, wire);

    /* we need to go on */
    wire = start;

  } while (wire != NULL)

}

/*
 * Try to aggregate all this information
 */
