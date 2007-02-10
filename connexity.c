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
	      const sited_pip_t *spip) {
  unsigned site_offset = site_index(spip->site);
  unsigned net_offset = spip->pip.source + site_offset * wiredb->dblen;
  debit_log(L_CONNEXITY, "returning net_offset value %i, site_offset %i", net_offset, site_offset);
  return net_offset;
}

static inline GNode *
net_of(GNode **db,
       const wire_db_t *wiredb,
       const chip_descr_t *chip,
       const sited_pip_t *pip) {
  unsigned index = net_offset_of(chip, wiredb, pip);
  return db[index];
}

static inline GNode *
net_register(GNode **db,
	     const wire_db_t *wiredb,
	     const chip_descr_t *chip,
	     const sited_pip_t *pip) {
  //  sited_wire_t *newwire = g_new(sited_wire_t, 1);
  sited_pip_t *newpip = g_slice_new(sited_pip_t);
  GNode *added = g_node_new(newpip);
  unsigned index = net_offset_of(chip, wiredb, pip);
  *newpip = *pip;
  db[index] = added;
  return added;
}

/*
 * This part is also in charge of doing default pip interpretation
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

/*
 * The sited wire is always a wire *startpoint*
 */

static GNode *
build_net_from(nets_t *nets,
	       GNode **nodetable,
	       const pip_db_t *pipdb,
	       const chip_descr_t *cdb,
	       const pip_parsed_dense_t *pipdat,
	       const sited_pip_t *spip_arg) {
  GNode *father, *child = NULL;
  wire_db_t *wiredb = pipdb->wiredb;
  sited_pip_t spip = *spip_arg;
  gboolean found;

  debit_log(L_CONNEXITY, "entering build_net_from");

  do {
    wire_atom_t pip_source = spip.pip.source;

#if 0 //DEBIT_DEBUG > 0
    gchar pipname[64];
    sprint_spip(pipname, pipdb->wiredb, &spip);
    debit_log(L_CONNEXITY, "iterating on spip %s", pipname);
#endif

    /* make a node out of the sited pip and register it, if needed */
    father = net_of(nodetable, wiredb, cdb, &spip);
    if (father) {
      debit_log(L_CONNEXITY, "GNode was already present");
      if (child) {
	debit_log(L_CONNEXITY, "linking to previous pipnode");
	g_node_prepend(father, child);
      }
      return father;
    }

    /* the pip is not yet present in the table, so we add it */
    father = net_register(nodetable, wiredb, cdb, &spip);
    if (child) {
      debit_log(L_CONNEXITY, "linking to previous pip");
      g_node_prepend(father, child);
    }

    /* We need the copper endpoint of the pip, which can be local with a
       pip locally driving the wire, or remote, when the wire startpoint
       is driven by a pip at the site startpoint of the wire */
    /* First, check that the wire is not locally-driven */
    /* Warning ! we're heavily relying on the fact that spip won't be
       touched if !found */
    found = get_interconnect_startpoint(pipdb, cdb, pipdat,
					&spip.pip.source, pip_source, spip.site);
    if (found) {
      debit_log(L_CONNEXITY, "pip is locally driven");
      spip.pip.target = pip_source;
      goto end_loop;
    }

    /* Second, check for a driver remotely */
    found = get_wire_startpoint(wiredb, cdb, &spip.site, &spip.pip.target, spip.site, spip.pip.source);
    if (!found) {
      debit_log(L_CONNEXITY, "leaving build_net_from, for lack of copper startpoint");
      return g_node_prepend(nets->head, father);
    }

    /* ask for the startpoint */
    found = get_interconnect_startpoint(pipdb, cdb, pipdat, &spip.pip.source, spip.pip.target, spip.site);
    if (!found) {
      debit_log(L_CONNEXITY, "leaving build_net_from, for lack of drivet at copper startpoint");
      return g_node_prepend(nets->head, father);
    }

    /* prepare to loop */
  end_loop:
    child = father;
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
  const pip_parsed_dense_t *pipdat;
} net_iterator_t;

static void
build_net_iter(gpointer data,
	       wire_atom_t start,
	       wire_atom_t end,
	       site_ref_t site) {
  net_iterator_t *arg = data;
  sited_pip_t spip = {
    .site = site,
    .pip = { .source = start,
	     .target = end, }
  };
  build_net_from(arg->nets, arg->nodetable,
		 arg->pipdb, arg->cdb, arg->pipdat, &spip);
}

static int
_build_nets(nets_t *nets,
	    const pip_db_t *pipdb,
	    const chip_descr_t *cdb,
	    const pip_parsed_dense_t *pipdat) {
  GNode **nodetable = alloc_wire_table(pipdb, cdb);
  net_iterator_t net_iter = {
    .nets = nets,
    .nodetable = nodetable,
    .pipdb = pipdb,
    .cdb = cdb,
    .pipdat = pipdat,
  };

  iterate_over_bitpips(pipdat, cdb, build_net_iter, &net_iter);

  g_free(nodetable);
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

static gboolean
print_wire(GNode *net,
	   gpointer data) {
  struct _print_net *arg = data;
  const wire_db_t *wiredb = arg->wiredb;
  const chip_descr_t *chip = arg->chipdb;
  gchar buf[64];
  sprint_spip(buf, wiredb, chip, net->data);
  g_print("%s ,\n", buf);
  return FALSE;
}

static void
print_net(GNode *net, gpointer data) {
  g_print("net %p {\n", net);
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
