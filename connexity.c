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

static inline
size_t lv_len(const chip_descr_t *chip) {
  return chip->width * LONGS_PER_SITE;
}

static inline
size_t lh_len(const chip_descr_t *chip) {
  return chip->height * LONGS_PER_SITE;
}

static inline GNode **
alloc_lv(const pip_db_t *pipdb, const chip_descr_t *chip) {
  size_t size = lv_len(chip);
  (void) pipdb;
  return g_new0(GNode *, size);
}

static inline GNode **
alloc_lh(const pip_db_t *pipdb, const chip_descr_t *chip) {
  size_t size = lh_len(chip);
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

/*
 * LH, LV hooks
 */
static inline unsigned
v_twists(const chip_descr_t *cdb,
	 const sited_wire_t swire) {
  const unsigned width = cdb->width;
  const unsigned v_index = site_index(swire.site) / width;
  const site_type_t type = site_type(cdb, swire.site);
  switch(type) {
#ifdef VIRTEX2
  case BTERM:
    return v_index - 1;
  case TTERM:
    return v_index + 1;
#endif
  default:
    return v_index;
  }
}

static inline unsigned
h_twists(const chip_descr_t *cdb,
	 const sited_wire_t swire) {
  const unsigned width = cdb->width;
  const unsigned h_index = site_index(swire.site)%width;
  const site_type_t type = site_type(cdb, swire.site);
  switch(type) {
#ifdef VIRTEX2
  case RTERM:
    return h_index - 1;
  case LTERM:
    return h_index + 1;
#endif
  default:
    return h_index;
  }
}

static inline unsigned
lv_offset_of(const wire_db_t *wiredb,
	     const chip_descr_t *cdb,
	     const sited_wire_t swire) {
  /* offset is the x index of site */
  const unsigned width = cdb->width;
  const unsigned v_offset = (v_twists(cdb, swire) + LONGS_PER_SITE
			     - wire_situation(wiredb, swire.wire)) % LONGS_PER_SITE;
  const unsigned offset = ((site_index(swire.site) % width) * LONGS_PER_SITE) + v_offset;
  return offset;
}

static inline unsigned
lh_offset_of(const wire_db_t *wiredb,
	     const chip_descr_t *cdb,
	     const sited_wire_t swire) {
  /* offset is the y index of site */
  const unsigned width = cdb->width;
  const unsigned h_offset = (h_twists(cdb, swire) + LONGS_PER_SITE
			     - wire_situation(wiredb, swire.wire)) % LONGS_PER_SITE;
  const unsigned offset = ((site_index(swire.site) / width) * LONGS_PER_SITE) + h_offset;
  return offset;
}

static inline void
long_register(GNode *driver,
	      const wire_type_t target_type,
	      const sited_wire_t swire,
	      const connexion_t *connexions,
	      const wire_db_t *wiredb,
	      const chip_descr_t *cdb) {
  unsigned index;

  switch (target_type) {
  case LV:
    index = lv_offset_of(wiredb, cdb, swire);
    connexions->lv[index] = driver;
    break;
  case LH:
    index = lh_offset_of(wiredb, cdb, swire);
    connexions->lh[index] = driver;
    break;
  default:
    g_assert_not_reached();
  }
}

static inline GNode *
long_register_fake(const wire_type_t target_type,
		   const sited_wire_t swire,
		   const connexion_t *connexions,
		   const wire_db_t *wiredb,
		   const chip_descr_t *cdb) {
  GNode *added = g_node_new(NULL);
  long_register(added, target_type, swire, connexions, wiredb, cdb);
  return added;
}

static inline GNode *
long_of(const wire_type_t target_type,
	const sited_wire_t swire,
	const connexion_t *connexions,
	const wire_db_t *wiredb,
	const chip_descr_t *cdb) {
  unsigned index;
  switch (target_type) {
  case LV:
    index = lv_offset_of(wiredb, cdb, swire);
    return connexions->lv[index];
  case LH:
    index = lh_offset_of(wiredb, cdb, swire);
    return connexions->lh[index];
  default:
    g_assert_not_reached();
  }
  return NULL;
}

static inline unsigned
gather(GNode **array, size_t array_len,
       GNode *head) {
  unsigned i, found = 0;
  for(i = 0; i < array_len; i++) {
    GNode *dummynet = array[i];
    if (dummynet && G_NODE_IS_ROOT(dummynet)) {
      GNode *net;
      while( (net = g_node_first_child(dummynet)) ) {
	g_node_unlink(net);
	g_node_prepend(head, net);
      }
      g_node_destroy(dummynet);
      found++;
    }
  }
  return found;
}

static unsigned
gather_longs(nets_t *nets,
	     const chip_descr_t *cdb,
	     const connexion_t *connexions) {
  GNode *head = nets->head;
  unsigned found = 0;

  found += gather(connexions->lv, lv_len(cdb), head);
  found += gather(connexions->lh, lh_len(cdb), head);

  if (found)
    g_warning("There were %i lonely longs found", found);

  return found;
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
net_register(GNode **db, GNode *allocd,
	     const wire_db_t *wiredb,
	     const sited_pip_t *pip) {
  sited_pip_t *newpip = g_slice_new(sited_pip_t);
  GNode *added = allocd ? allocd : g_node_new(newpip);
  unsigned index = net_offset_of(wiredb, pip);
  added->data = newpip;
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
	      const connexion_t *connexions,
	      const wire_db_t *wiredb,
	      const chip_descr_t *cdb) {
  gboolean present = TRUE;
  wire_type_t target_type = wire_type(wiredb, spip->pip.target);
  sited_wire_t swire = { .wire = spip->pip.target, .site = spip->site };
  GNode **nodetable = connexions->nodetable;
  GNode *cached = NULL, *father = net_of(nodetable, wiredb, spip);

  g_assert(spip->pip.target != WIRE_EP_END);

  switch (target_type) {
  case LH:
  case LV: {
    /* A GNode may already have been allocated while visiting the wires
       driven by the long. We use it if present -- and we should also do
       some checks there in fact */
    /* lookup source ! */
    if (spip->pip.source != WIRE_EP_END)
      cached = long_of(target_type, swire, connexions, wiredb, cdb);
    else
      cached = NULL;

    break;
  }
  default:
    cached = NULL;
  }

  /* In case the pip is not yet present in the table, add it */
  if (!father) {
    present = FALSE;
    father = net_register(nodetable, cached, wiredb, spip);
  }

  if (driven)
    g_node_prepend(father, driven);

  switch (target_type) {
  case LH:
  case LV: {
    /* This wire should also be registered as driven by the correct LV/LH
       We'll creating the GNode if not already present */
    GNode *driver = long_of(target_type, swire, connexions, wiredb, cdb);

    if (spip->pip.source != WIRE_EP_END) {
      long_register(father, target_type, swire, connexions, wiredb, cdb);
      break;
    }

    if (driver == NULL)
      driver = long_register_fake(target_type, swire, connexions, wiredb, cdb);

    if (!present) {
      g_node_prepend(driver, father);
      present = TRUE;
    }
    break;
  }
  default:
    break;
  }

  *added = father;
  return present;
}

static inline gboolean
get_pip_startpoint(wire_atom_t *source,
		   const wire_atom_t target,
		   const pip_db_t *pipdb,
		   const chip_descr_t *cdb,
		   const site_ref_t site,
		   const pip_parsed_dense_t *pipdat) {

  return
    get_interconnect_startpoint(pipdat, source, target, site) ||
    get_implicit_startpoint(source, pipdb, cdb, target, site);
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
get_wire_driver(const pip_db_t *pipdb,
		const chip_descr_t *cdb,
		const pip_parsed_dense_t *pipdat,
		sited_pip_t *driver) {
  gboolean found;
  const wire_db_t *wiredb = pipdb->wiredb;
  wire_type_t wtype = wire_type(wiredb,driver->pip.source);

  /*
   * Long wires -- do something clever !
   */
  switch(wtype) {
  case LH: {
    /* Register the wire in the table, and end the search. We're betting
       on the wire being driven from another place. We'll lose the wire
       end if the long wire has no driver. This could be detected
       *after* all operations, in a kind of DRC check */
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

  debit_log(L_CONNEXITY, "wire driver from source %s and target %s",
	    wire_name(pipdb->wiredb, driver->pip.source),
	    wire_name(pipdb->wiredb, driver->pip.target));

  found =
    get_wire_startpoint(wiredb, cdb, &driver->site, &driver->pip.target, driver->site, driver->pip.source) &&
    get_pip_startpoint(&driver->pip.source, driver->pip.target, pipdb, cdb, driver->site, pipdat);

  debit_log(L_CONNEXITY, "wire source %s and target %s in the end",
	    wire_name(pipdb->wiredb, driver->pip.source),
	    wire_name(pipdb->wiredb, driver->pip.target));

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
  wire_db_t *wiredb = pipdb->wiredb;
  sited_pip_t spip = *spip_arg;
  GNode *newnode = NULL;
  gboolean found;

  debit_log(L_CONNEXITY, "entering build_net_from");

  do {
    wire_atom_t pip_source = spip.pip.source;
    gboolean exists;

    /* make a node out of the sited pip and register it, if needed */
    exists = register_spip(&newnode, &spip, newnode, connexions, wiredb, cdb);
    if (exists)
      return newnode;

    /* Try to find the next pip */
    spip.pip.target = pip_source;
    g_assert(spip.pip.target != WIRE_EP_END);

    /* We need the copper endpoint of the pip, which can be local with a
       pip locally driving the wire, or remote, when the wire startpoint
       is driven by a pip at the site startpoint of the wire */
    /* First, check that the wire is not locally-driven -- that is,
       the source of the pip is driven locally by another pip at the
       same site */
    /* XXX replace with get_pip */
    found = get_pip_startpoint(&spip.pip.source, pip_source, pipdb, cdb, spip.site, pipdat);
    //    found = get_interconnect_startpoint(pipdat, &spip.pip.source, pip_source, spip.site);
    g_assert(spip.pip.target != WIRE_EP_END);
    if (found)
      continue;

    /* The source wire is not driven by another pip at the local
       site. Then we consult the wiring database to first get to another
       site, and then check the pips there. */

    /* Get to the other site -- replace site and targets in the spip
       structure with values found. */
    found = get_wire_driver(pipdb, cdb, pipdat, &spip);
  } while (found);

  /* Add a dummy pip to record the absence of driver */
  g_assert(spip.pip.target != WIRE_EP_END);
  spip.pip.source = WIRE_EP_END;
  if (!register_spip(&newnode, &spip, newnode, connexions, wiredb, cdb))
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

  /* Gather unconnected LV / LH wires, and put them again into
     the net structure */
  gather_longs(nets, cdb, connex);

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

static
gboolean free_net(GNode *node,
		  gpointer data) {
  (void) data;
  if (node->data)
    g_slice_free(sited_pip_t, node->data);
  return FALSE;
}

void free_nets(nets_t *nets) {
  GNode *root = nets->head;
  g_node_traverse (root, G_IN_ORDER, G_TRAVERSE_ALL, -1, free_net, NULL);
  g_node_destroy(root);
  g_free(nets);
}
