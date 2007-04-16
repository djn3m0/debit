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

typedef struct _connexion {
  /* GNode table, for gathering pips in an organized fashion */
  GNode **nodetable;
  /* GNode table, for gathering pips at long wires */
  GNode **lv, **lh;
} connexion_t;

/* For now be dumb */
static inline GNode **
alloc_wire_table(const pip_db_t *pipdb, const chip_descr_t *chip) {
  gsize size = pipdb->wiredb->dblen * chip->width * chip->height;
  /* we could divide the size by two by only storing startpoint
     in this table */
  return g_new0(GNode *, size);
}

#define LONGS_PER_SITE 24

static inline GNode **
alloc_lv(const pip_db_t *pipdb, const chip_descr_t *chip) {
  size_t size = chip->width * LONGS_PER_SITE;
  (void) pipdb;
  return g_new0(GNode *, size);
}

static inline GNode **
alloc_lh(const pip_db_t *pipdb, const chip_descr_t *chip) {
  size_t size = chip->height * LONGS_PER_SITE;
  (void) pipdb;
  return g_new0(GNode *, size);
}

static connexion_t *
alloc_connexions(const pip_db_t *pipdb, const chip_descr_t *chip) {
  connexion_t *connexions = g_new(connexion_t, 1);
  connexions->nodetable = alloc_wire_table(pipdb, chip);
  connexions->lv = alloc_lv(pipdb, chip);
  connexions->lh = alloc_lh(pipdb, chip);
  return connexions;
}

static void
free_connexions(connexion_t *connexions) {
  g_free(connexions->nodetable);
  g_free(connexions->lv);
  g_free(connexions->lh);
  g_free(connexions);
  return;
}

/* Be dumb again */
static inline unsigned
net_offset_of(const wire_db_t *wiredb,
	      const sited_pip_t *spip) {
  unsigned site_offset = site_index(spip->site);
  unsigned net_offset = spip->pip.target + site_offset * wiredb->dblen;
  debit_log(L_CONNEXITY, "returning net_offset value %i, site_offset %i", net_offset, site_offset);
  return net_offset;
}

static inline GNode *
net_of(GNode **db,
       const wire_db_t *wiredb,
       const sited_pip_t *pip) {
  unsigned index = net_offset_of(wiredb, pip);
  return db[index];
}

static inline GNode *
net_register(GNode **db,
	     const wire_db_t *wiredb,
	     const sited_pip_t *pip) {
  //  sited_wire_t *newwire = g_new(sited_wire_t, 1);
  sited_pip_t *newpip = g_slice_new(sited_pip_t);
  GNode *added = g_node_new(newpip);
  unsigned index = net_offset_of(wiredb, pip);
  *newpip = *pip;
  db[index] = added;
  return added;
}

/*
 * This part is also in charge of doing default pip interpretation
 */

/*
 * Register a wire
 */

static inline gboolean
register_spip(GNode **added,
	      const sited_pip_t *spip,
	      GNode *driven,
	      GNode **nodetable,
	      const wire_db_t *wiredb) {
  gboolean present = TRUE;
  GNode *father = net_of(nodetable, wiredb, spip);

  /* in case the pip is not yet present in the table,
     add it */
  if (!father) {
    present = FALSE;
    father = net_register(nodetable, wiredb, spip);
  }

  if (driven) {
    debit_log(L_CONNEXITY, "linking to previous pip");
    g_node_prepend(father, driven);
  }

  *added = father;
  return present;
}

/*
 * Try to find a remote drive for the wire at hand.
 *
 * There's a problem here for wires which can be set from multiple
 * locations -- we'll need to iterate on all possible locations and see
 * who's driving. Sometimes this can be simply put in a database. In our
 * first example, though -- long wires --, this cannot be handled simply.
 */

static inline gboolean
get_wire_driver(const wire_db_t *wiredb,
		const chip_descr_t *cdb,
		const pip_parsed_dense_t *pipdat,
		sited_pip_t *driver) {
  gboolean found;
  wire_type_t wtype = wire_type(wiredb,driver->pip.source);

  /*
   * Long wires -- do something clever !
   */
  switch(wtype) {
  case LH: {
    return FALSE;
  }
  case LV: {
    return FALSE;
  }
  /*
   * Standard wire: get the remote endpoing, which is read from the
   * database, then the pip driving the endpoint
   */
  default:
    break;
  }
  found =
    get_wire_startpoint(wiredb, cdb, &driver->site, &driver->pip.target, driver->site, driver->pip.source) &&
    get_interconnect_startpoint(pipdat, &driver->pip.source, driver->pip.target, driver->site);

  return found;
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

static GNode *
build_net_from(nets_t *nets,
	       connexion_t *connexions,
	       const pip_db_t *pipdb,
	       const chip_descr_t *cdb,
	       const pip_parsed_dense_t *pipdat,
	       const sited_pip_t *spip_arg) {
  GNode **nodetable = connexions->nodetable;
  wire_db_t *wiredb = pipdb->wiredb;
  sited_pip_t spip = *spip_arg;
  GNode *newnode = NULL;
  gboolean found;

  debit_log(L_CONNEXITY, "entering build_net_from");

  do {
    wire_atom_t pip_source = spip.pip.source;
    gboolean exists;

    /* make a node out of the sited pip and register it, if needed */
    exists = register_spip(&newnode, &spip, newnode, nodetable, wiredb);
    if (exists)
      return newnode;

    /* Try to find the next pip */
    spip.pip.target = pip_source;

    /* We need the copper endpoint of the pip, which can be local with a
       pip locally driving the wire, or remote, when the wire startpoint
       is driven by a pip at the site startpoint of the wire */
    /* First, check that the wire is not locally-driven -- that is,
       the source of the pip is driven locally by another pip at the
       same site */
    found = get_interconnect_startpoint(pipdat, &spip.pip.source, pip_source, spip.site);
    if (found)
      continue;

    /* The source wire is not driven by another pip at the local
       site. Then we consult the wiring database to first get to another
       site, and then check the pips there. */

    /* Get to the other site -- replace site and targets in the spip
       structure with values found. */
    found = get_wire_driver(wiredb, cdb, pipdat, &spip);

  } while (found);

  /* Add a dummy pip to record the absence of driver */
  spip.pip.source = WIRE_EP_END;
  if (!register_spip(&newnode, &spip, newnode, nodetable, wiredb))
    return g_node_prepend(nets->head, newnode);

  return newnode;
}

/*
 *
 */

typedef struct _net_iterator {
  nets_t *nets;
  connexion_t *connexions;
  const pip_db_t *pipdb;
  const chip_descr_t *cdb;
  const pip_parsed_dense_t *pipdat;
} net_iterator_t;

static void
build_net_iter(gpointer data, const pip_t pip, const site_ref_t site) {
  net_iterator_t *arg = data;
  const sited_pip_t spip = {
    .site = site,
    .pip = pip,
  };

  build_net_from(arg->nets, arg->connexions,
		 arg->pipdb, arg->cdb, arg->pipdat, &spip);
}

static int
_build_nets(nets_t *nets,
	    const pip_db_t *pipdb,
	    const chip_descr_t *cdb,
	    const pip_parsed_dense_t *pipdat) {
  connexion_t *connex = alloc_connexions(pipdb, cdb);
  net_iterator_t net_iter = {
    .nets = nets,
    .connexions = connex,
    .pipdb = pipdb,
    .cdb = cdb,
    .pipdat = pipdat,
  };

  iterate_over_bitpips(pipdat, cdb, build_net_iter, &net_iter);

  free_connexions(connex);
  return 0;
}

nets_t *build_nets(const pip_db_t *pipdb,
		   const chip_descr_t *cdb,
		   const pip_parsed_dense_t *pipdat) {
  nets_t *ret = g_new(nets_t, 1);
  int err;

  ret->head = g_node_new(NULL);

  err = _build_nets(ret, pipdb, cdb, pipdat);
  if (err) {
    g_free(ret);
    return NULL;
  }

  return ret;
}

void free_nets(nets_t *nets) {
  g_node_destroy(nets->head);
  g_free(nets);
}

/* Printing function */
struct _print_net {
  const wire_db_t *wiredb;
  const chip_descr_t *chipdb;
};

#ifdef VIRTEX2

static const gchar *typenames[NR_WIRE_TYPE] = {
  [BX] = "BX",
  [BY] = "BY",
  [X] = "X",
  [XB] = "XB",
  [XQ] = "XQ",
  [Y] = "Y",
  [YB] = "YB",
  [YQ] = "YQ",
  [F1] = "F1",
  [F2] = "F2",
  [F3] = "F3",
  [F4] = "F4",
  [G1] = "G1",
  [G2] = "G2",
  [G3] = "G3",
  [G4] = "G4",
  [CE] = "CE",
  [SR] = "SR",
};

static inline
const char *typename(const wire_type_t wt) {
  const gchar *name = typenames[wt];
  return name ? name : "unknown";
}

typedef enum iopin_dir {
  IO_INPUT,
  IO_OUTPUT,
  IO_END,
} iopin_dir_t;

const char *ioname[IO_END] = {
  [IO_INPUT] = "inpin",
  [IO_OUTPUT] = "outpin",
};

static void
print_iopin(const iopin_dir_t iodir,
	    const sited_pip_t *spip,
	    const wire_db_t *wiredb,
	    const chip_descr_t *chip) {
  /* Use type to get the name of the wire in the instance */
  const wire_t *wire = get_wire(wiredb, spip->pip.target);
  const csite_descr_t *site = get_site(chip, spip->site);
  gchar slicen[MAX_SITE_NLEN];
  snprint_slice(slicen, MAX_SITE_NLEN, chip, site, wire->situation - ZERO);
  /* Combine the situation and site to get the location */
  g_print("%s \"%s\" %s,\n", ioname[iodir], slicen, typename(wire->type));
}

static gboolean
print_inpin(GNode *net,
	    gpointer data) {
  const sited_pip_t *spip = net->data;
  struct _print_net *arg = data;
  print_iopin(IO_INPUT, spip, arg->wiredb, arg->chipdb);
  return FALSE;
}

static gboolean
print_outpin(GNode *net,
	     gpointer data) {
  const sited_pip_t *spip = net->data;
  struct _print_net *arg = data;
  print_iopin(IO_OUTPUT, spip, arg->wiredb, arg->chipdb);
  return FALSE;
}

#else

static gboolean
print_inpin(GNode *net,
	    gpointer data) {
  (void) net;
  (void) data;
  return FALSE;
}

static gboolean
print_outpin(GNode *net,
	     gpointer data) {
  (void) net;
  (void) data;
  return FALSE;
}

#endif

static gboolean
print_wire(GNode *net,
	   gpointer data) {
  struct _print_net *arg = data;
  const wire_db_t *wiredb = arg->wiredb;
  const chip_descr_t *chip = arg->chipdb;
  const sited_pip_t *spip = net->data;
  gchar buf[64];

  /* This is how wire start are indicated -- this is actually redundant
     with positioning in the tree... */
  if (spip->pip.source == WIRE_EP_END)
    return FALSE;

  sprint_spip(buf, wiredb, chip, net->data);
  g_print("%s ,\n", buf);
  return FALSE;
}

static void
print_net(GNode *net, gpointer data) {
  static unsigned netnum = 0;
  g_print("net %i {\n", netnum++);
  /* print input -- this should be the output pin of a logical bloc */
  print_outpin(net, data);
  /* print outputs -- these should be input pins to some logical blocs */
  g_node_traverse (net, G_IN_ORDER, G_TRAVERSE_LEAVES, -1, print_inpin, data);
  g_node_traverse (net, G_PRE_ORDER, G_TRAVERSE_ALL, -1, print_wire, data);
  g_print("}\n");
}

void print_nets(nets_t *net,
		const pip_db_t *pipdb,
		const chip_descr_t *cdb) {
  struct _print_net arg = { .wiredb = pipdb->wiredb, .chipdb = cdb };
  /* Iterate through nets */
  g_node_children_foreach (net->head, G_TRAVERSE_ALL, print_net, &arg);
}
