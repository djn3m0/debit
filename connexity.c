/*
 * (C) Copyright 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
 * All rights reserved.
 */

#include <glib.h>
#include "debitlog.h"

#include "wiring.h"
#include "localpips.h"
#include "connexity.h"

/*
 * Connexity analysis
 */

/* For now be dumb */
static inline GNode **
alloc_wire_table(const pip_db_t *pipdb, const chip_descr_t *chip) {
  gsize size = pipdb->wiredb->dblen * chip->width * chip->height;
  /* we could divide the size by two by only storing startpoint
     in this table */
  return g_new0(GNode *, size);
}

/* Be dumb again */
static inline unsigned
net_offset_of(const chip_descr_t *chip,
	      const wire_db_t *wiredb,
	      const sited_wire_t *wire) {
  unsigned site_offset = wire->site - chip->data;
  g_print("net_offset_of is %i\n", site_offset);
  return wire->wire + site_offset * wiredb->dblen;
}

static inline GNode *
net_of(GNode **db,
       const wire_db_t *wiredb,
       const chip_descr_t *chip,
       const sited_wire_t *wire) {
  unsigned index = net_offset_of(chip, wiredb, wire);
  return db[index];
}

static inline GNode *
net_register(GNode **db,
	     const wire_db_t *wiredb,
	     const chip_descr_t *chip,
	     const sited_wire_t *wire) {
  sited_wire_t *newwire = g_new(sited_wire_t, 1);
  GNode *added = g_node_new(newwire);
  unsigned index = net_offset_of(chip, wiredb, wire);
  *newwire = *wire;
  db[index] = added;
  return added;
}

/*
 * This part is also in charge of doing default pip interpretation
 */


/** \brief Structure describing all nets in an FPGA
 *
 * This structure is an N-ary tree. The first-level nodes are the
 * different nets; then from then on
 *
 * The data in the nodes are sited wires.
 */

static inline  GNode *
net_add(GNode *net, GNode **nodetable,
	const wire_db_t *wiredb,
	const chip_descr_t *chip,
	const sited_wire_t *wire) {
  GNode *added = net_register(nodetable, wiredb, chip, wire);
  g_node_append(net, added);
  return added;
}

static inline GNode *
add_new_net(nets_t *nets, GNode **nodetable,
	    const wire_db_t *wiredb,
	    const chip_descr_t *chip,
	    const sited_wire_t *wire) {
  return net_add(nets->head, nodetable, wiredb, chip, wire);
}

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

/*
 * The sited wire is always a wire *startpoint*
 */

static inline
gboolean get_endpoint(sited_wire_t *start,
		      const pip_db_t *pipdb,
		      const chip_descr_t *cdb,
		      const bitstream_parsed_t *bitstream,
		      const sited_wire_t *wire) {
  g_print("getting startpoint of wire %s\n",
	  wire_name(pipdb->wiredb, wire->wire));

  return (get_interconnect_startpoint(pipdb, bitstream, start, wire) ||
	  get_wire_startpoint(pipdb->wiredb, cdb, start, wire));
}

static GNode *
build_net_from(nets_t *nets,
	       GNode **nodetable,
	       const pip_db_t *pipdb,
	       const chip_descr_t *cdb,
	       const bitstream_parsed_t *bitstream,
	       const sited_wire_t *wire) {
  GNode *father, *child = NULL;
  wire_db_t *wiredb = pipdb->wiredb;
  gboolean found;
  sited_wire_t orig = *wire, start;

  do {
    /* make a node out of this and register this. If the node is already
       in there, return it */
    debit_log(L_CONNEXITY, "iterating on startpoint of wire %s\n",
	      wire_name(wiredb, orig.wire));

    father = net_of(nodetable, wiredb, cdb, &orig);
    if (father) {
      if (child)
	g_node_append(father, child);
      return father;
    }

    /* else create the node and add a child */
    father = net_register(nodetable, wiredb, cdb, &orig);
    if (child)
	g_node_append(father, child);

    /* Then the connexity analysis proper. We're relying on the
       bitstream data directly to do this. We could also have some
       function fill in directly the GNode ** structure then have
       this loop connect everything. It would probably be much better. */
    debit_log(L_CONNEXITY, "getting endpoint of wire %s\n",
	      wire_name(wiredb, start.wire));
    found = get_endpoint(&start, pipdb, cdb, bitstream, &orig);

    if (!found)
      return add_new_net(nets, nodetable, wiredb, cdb, &orig);

    /* prepare to loop */
    orig = start;

  } while (father != NULL);

  g_assert_not_reached();
  return NULL;
}

/*
 *
 */

typedef struct _net_iterator {
  nets_t *nets;
  GNode **nodetable;
  const pip_db_t *pipdb;
  const chip_descr_t *cdb;
  const bitstream_parsed_t *bitstream;
} net_iterator_t;

static void
build_net_iter(gpointer data,
	       wire_atom_t start,
	       wire_atom_t end,
	       site_ref_t site) {
  net_iterator_t *arg = data;
  sited_wire_t wire = {
    .wire = end,
    .site = site,
  };
  build_net_from(arg->nets, arg->nodetable,
		 arg->pipdb, arg->cdb, arg->bitstream,
		 &wire);
}

static void
net_site_iter(unsigned x, unsigned y, site_ref_t site,
	      gpointer dat) {
  net_iterator_t *arg = dat;
  iterate_over_bitpips(arg->pipdb, arg->bitstream, site,
		       build_net_iter, dat);
}

static int
_build_nets(nets_t *nets,
	    const pip_db_t *pipdb,
	    const chip_descr_t *cdb,
	    const bitstream_parsed_t *bitstream) {
  GNode **nodetable = alloc_wire_table(pipdb, cdb);
  net_iterator_t net_iter = {
    .nets = nets,
    .nodetable = nodetable,
    .pipdb = pipdb,
    .cdb = cdb,
    .bitstream = bitstream,
  };

  iterate_over_sites(cdb, net_site_iter, &net_iter);

  g_free(nodetable);
  return 0;
}

nets_t *build_nets(const pip_db_t *pipdb,
		   const chip_descr_t *cdb,
		   const bitstream_parsed_t *bitstream) {
  nets_t *ret = g_new(nets_t, 1);
  int err;

  ret->head = g_node_new(NULL);

  err = _build_nets(ret, pipdb, cdb, bitstream);
  if (err) {
    g_free(ret);
    return NULL;
  }

  return ret;
}

void free_nets(nets_t *nets) {
  g_free(nets->head);
  g_free(nets);
}

/* Printing function */
struct _print_net {
  const pip_db_t *pipdb;
  const chip_descr_t *cdb;
};

static gboolean
print_wire(GNode *net,
	   gpointer data) {
  struct _print_net *arg = data;
  sited_wire_t *wire = net->data;
  gchar buf[30];
  /* do something ! */
  (void) arg;
  sprint_csite(buf, wire->site);
  g_print("\nwire %i @%s\n", wire->wire, buf);
  return FALSE;
}

static void
print_net(GNode *net, gpointer data) {
  g_print("net %p {\n", net);
  g_node_traverse (net, G_IN_ORDER, G_TRAVERSE_ALL, -1, print_wire, data);
  g_print("}\n");
}

void print_nets(nets_t *net,
		const pip_db_t *pipdb,
		const chip_descr_t *cdb) {
  struct _print_net arg = { .pipdb = pipdb, .cdb = cdb };
  /* Iterate through nets */
  g_node_children_foreach (net->head, G_TRAVERSE_ALL, print_net, &arg);
}
