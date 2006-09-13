/*
 * (C) Copyright 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
 * All rights reserved.
 */

/** \file
 *
 *  Idea to rewrite the DB:
 *  add one entry bitlist in the bitlist, which lists all the
 *  configuration bits. Then a pip is given as an integer (guint32).
 *  Fine, no ? Checking a bit is just getting the data into a
 *  guint32, and comparing it to the entry in the DB. Fine !
 *
 *  Also, as an optimisation we can get back the data as
 *  startpoint, length, width of the square.
 *
 */

/* implements the functions needed to get pips in the db */

#define G_LOG_DOMAIN  "LOCALPIPS"
#include <glib.h>

#include "keyfile.h"

#include "bitstream.h"
#include "localpips.h"
#include "wiring.h"

#define STRINGCHUNK_DEFAULT_SIZE 16

/*
 * Forward declarations
 */

static int build_datatree_from_keyfiles(GKeyFile *data, GKeyFile *control,
					wire_db_t *wires, GNode *head);
static void destroy_datatree(GNode *head);

static const gchar *basedbnames[SITE_TYPE_NEUTRAL] = {
  [CLB] = "clb",
  [TTERM] = "tterm",
  [BTERM] = "bterm",
  [RTERM] = "rterm",
  [LTERM] = "lterm",
  [BIOI] = "bioi",
  [TIOI] = "tioi",
  [RIOI] = "rioi",
  [LIOI] = "lioi",
  [BRAM] = "bram",
  [BTERMBRAM] = "btermbram",
  [BIOIBRAM] = "bioibram",
  [TIOIBRAM] = "tioibram",
};

/** \brief Read a database from files
 *
 * Readback a set of files describing a pip database and fills in the
 * pip_db_t structure, in-code view of the database.
 *
 * @param pipdb the already-alloced pipdb structure to be filled in
 * @param control the name of the control db file to read
 * @param data the name of the data db file to read
 *
 * @return the status of the operation
 * @see pip_db_t
 */

static int
read_db_from_file(pip_db_t *pipdb, const gchar *datadir) {
  int err = 0;
  gchar *filename = NULL;
  GKeyFile *control = NULL, *data = NULL;
  guint i;

  for (i = 0; i < SITE_TYPE_NEUTRAL; i++) {
    const gchar *base = basedbnames[i];

    if (!base)
      continue;

    filename = g_build_filename(datadir,base,"control.db",NULL);
    err = read_keyfile(&control,filename);
    g_free(filename);
    if (err)
      goto out_err_free;

    filename = g_build_filename(datadir,base,"data.db",NULL);
    err = read_keyfile(&data,filename);
    g_free(filename);
    if (err)
      goto out_err_free;

    pipdb->memorydb[i] = g_node_new(NULL);
    err = build_datatree_from_keyfiles(data, control,
				       pipdb->wiredb,
				       pipdb->memorydb[i]);
    if (err)
      goto out_err_free;

    g_key_file_free(control);
    g_key_file_free(data);
    control = NULL;
    data = NULL;
  }

  return 0;

 out_err_free:
  if (control)
    g_key_file_free(control);
  if (data)
    g_key_file_free(data);
  free_pipdb (pipdb);
  return err;
}

/** \brief Allocate and fill a pip database
 *
 * Load a new database into memory
 */
pip_db_t *
get_pipdb(const gchar *datadir) {
  pip_db_t *ret;

  ret = g_new0(pip_db_t, 1);

  /* XXX This to be passed as argument in final version of API */
  ret->wiredb = get_wiredb(datadir);
  if (!ret->wiredb) {
    g_free(ret);
    return NULL;
  }

  if (read_db_from_file(ret,datadir))
    return NULL;

  return ret;
}

/** \brief Free a filled pip database
 *
 * Free all structures allocated during the database loading
 *
 * @param pipdb the filled in pipdb structure to be freed
 * @see read_db_from_file
 * @see pip_db_t
 */

void
free_pipdb(pip_db_t *pipdb) {
  guint i;

  /* XXX This to be done elsewhere final version of API */
  if (pipdb->wiredb)
    free_wiredb(pipdb->wiredb);

  for(i = 0; i < SITE_TYPE_NEUTRAL; i++) {
    if (pipdb->memorydb[i])
      destroy_datatree(pipdb->memorydb[i]);
    pipdb->memorydb[i] = NULL;
  }

  g_free(pipdb);
}

/** \brief Iterator over endpoint nodes in memory db
 */

static inline void
iterate_over_groups_memory(GNode *head,
			   GNodeForeachFunc func,
			   gpointer data) {
  g_node_children_foreach(head,G_TRAVERSE_ALL,func,data);
}

typedef void (*pip_hook_t)(gpointer, const gchar *, const gchar *);

static void
iterate_over_starts(GKeyFile *db, pip_hook_t func, gpointer closure,
		    const gchar *endpoint)
{
  gchar **starts;
  gsize nstarts, j;

  starts = g_key_file_get_keys(db, endpoint, &nstarts, NULL);
  if (!starts) {
    g_error("Error getting keys from db for endpoint %s",endpoint);
    return;
  }

  for (j = 0; j < nstarts; j++) {
    const gchar *start = starts[j];
    /* we could need continuations there */
    func(closure,start,endpoint);
  }

  g_strfreev(starts);
  return;
}

typedef struct _pip_iterator {
  pip_hook_t pip_iterator;
  gpointer closure_iterator;
} pip_iterator_t;

/* Serialized version of the function */

static void
iterate_over_starts_hook(GKeyFile *datadb, const gchar *endpoint, gpointer data)
{
  pip_iterator_t *iter = data;

  iterate_over_starts(datadb,
		      iter->pip_iterator,
		      iter->closure_iterator,
		      endpoint);
}

/** \brief Iterator over pips.
 *
 * This function iterates a function over all pips in the database; it
 * uses the iterator over group above
 *
 * @param pipdb the pip database to iterate over
 * @param func the function to iterate on each pip
 * @param closure the closure argument of the iterating function
 * @see pip_hook_t
 */

static void
iterate_over_pips(GKeyFile *db,
		  pip_hook_t func,
		  gpointer closure) {
  pip_iterator_t localclosure;
  localclosure.pip_iterator = func;
  localclosure.closure_iterator = closure;
  iterate_over_groups(db, iterate_over_starts_hook, &localclosure);
}

/**
 * Helper functions for building the in-memory representation of the
 * file data.
 */

typedef struct _localpip_data {
  wire_atom_t startwire;
  guint32 cfgdata;
} localpip_data_t;

typedef struct _build_wirenode {
  GKeyFile *datadb;
  GNode *groupnode;
  wire_db_t *wires;
} build_wirenode_t;

static guint32 get_pip_data_from_file(GKeyFile *keyfile,
				      const gchar *start,
				      const gchar *end);

void build_wirenode(gpointer data, const gchar *start, const gchar *end) {
  build_wirenode_t *exam = data;
  localpip_data_t *dat = g_new(localpip_data_t, 1);
  GNode *wirenode;

  if (parse_wire_simple(exam->wires, &dat->startwire, start)) {
    g_warning("unparsable wire %s", start);
    g_free(dat);
    return;
  }
  dat->cfgdata = get_pip_data_from_file(exam->datadb, start, end);

  wirenode = g_node_new(dat);
  g_node_append(exam->groupnode, wirenode);
}

typedef struct _localpip_control_data {
  wire_atom_t endwire;
  gsize size;
  guint32 *data;
} localpip_control_data_t;

typedef struct _build_groupnode {
  GKeyFile *ctrldb;
  wire_db_t *wires;
  GNode *head;
} build_groupnode_t;

static inline gint *get_pip_structure_from_file(GKeyFile *keyfile,
						const gchar *end,
						gsize *length);

static void build_groupnode(GKeyFile *datadb, const gchar* endp,
			    gpointer data) {
  build_groupnode_t *exam = data;
  wire_db_t *wires = exam->wires;
  localpip_control_data_t *dat = g_new(localpip_control_data_t, 1);
  GNode *groupnode;
  build_wirenode_t arg = { .datadb = datadb, .wires = wires };

  if (parse_wire_simple(wires, &dat->endwire, endp)) {
    g_warning("unparsable wire %s", endp);
    g_free(dat);
    return;
  }

  dat->data = (guint *)get_pip_structure_from_file(exam->ctrldb, endp, &dat->size);

  groupnode = g_node_new(dat);
  g_node_append(exam->head, groupnode);
  arg.groupnode = groupnode;

  iterate_over_starts(datadb, build_wirenode, &arg, endp);
}

/** \brief Convert ini file data into optimized in-memory representation
 *
 *
 *
 */
static int
build_datatree_from_keyfiles(GKeyFile *data, GKeyFile *control,
			     wire_db_t *wires, GNode *head) {
  build_groupnode_t arg = {
    .ctrldb = control,
    .head = head,
    .wires = wires,
  };

  iterate_over_groups(data, build_groupnode, &arg);
  return 0;
}



/** \brief Release memory database
 *
 *
 */

static void release_groupnode (GNode *node,
			       gpointer data) {
  localpip_control_data_t *dat = node->data;
  (void) data;
  g_free(dat->data);
  dat->data = NULL;
}

static gboolean release_node (GNode *node,
			      gpointer data) {
  void *dat = node->data;
  (void) data;
  if (dat)
    g_free(dat);
  node->data = NULL;
  return FALSE;
}

static void
destroy_datatree(GNode *head) {
  iterate_over_groups_memory(head, release_groupnode, NULL);
  g_node_traverse(head, G_IN_ORDER, G_TRAVERSE_ALL, -1, release_node, NULL);
  g_node_destroy(head);
  return;
}

/*
 * Democode, print the DB
 */

static void
print_pip_hook(gpointer data, const gchar *start, const gchar *end) {
  (void) data;
  g_log(G_LOG_DOMAIN, PIP_LOG_DATA, "pip %s -> %s", start, end);
}

/** \brief Print pip database.
 *
 * This simple function uses the iterator mechanism to print the whole db.
 *
 * @param pipdb the pip database to print
 */
void print_pipdb(GKeyFile *pipdb)
{
  iterate_over_pips(pipdb, print_pip_hook, NULL);
}

/** \brief Raw query of the file containing the pip data database
 *
 * This function returns the site-independent database used to identify
 * a pip in the bitstream.
 *
 * @param pipdb pip database
 * @param start pip startpoint
 * @param end pip endpoint
 *
 * @return the characteristic value of the endpoint data for the pip
 */
static guint32 get_pip_data_from_file(GKeyFile *keyfile,
				      const gchar *start,
				      const gchar *end) {
  return g_key_file_get_integer(keyfile, end, start, NULL);
}

/** \brief Raw query of the file representing the pip control database
 *
 * This function returns the site-independent control information
 * present in the database and directly used to locate the configuration
 * bits of the pip in the bitstream.
 *
 * @param pipdb pip database
 * @param end pip endpoint
 * @param length pointer to variable holding the number of bits part of
 * the returned configuration
 *
 * @return the list of bits
 *
 */

static inline gint *get_pip_structure_from_file(GKeyFile *keyfile,
						const gchar *end,
						gsize *length) {
  return g_key_file_get_integer_list(keyfile, end, "BITLIST", length, NULL);
}

typedef struct _examine_data_memory {
  guint32 bitstream_data;
  /* at some point get rid of wiredb and endwire */
  wire_db_t *wiredb;
  wire_atom_t endwire;
  wire_atom_t startwire;
  gboolean found;
} examine_data_memory_t;

static void examine_node_memory (GNode *node,
				 gpointer data) {
  examine_data_memory_t *exam_arg = data;
  localpip_data_t *cfgarg = node->data;
  wire_db_t *wiredb = exam_arg->wiredb;
  const gchar *end = wire_name(wiredb,exam_arg->endwire);
  const gchar *start = wire_name(wiredb,cfgarg->startwire);
  const guint32 cfgdata = cfgarg->cfgdata;
  const guint32 bitdata = exam_arg->bitstream_data;

  /* Please keep these warnings on, they indicate debitting
     idiosyncrasies that we don't fully get yet */
  if ( bitdata == cfgdata ) {
    if (exam_arg->found)
      g_warning("Perfect match replacing %s -> %s, with %s -> %s",
		wire_name(wiredb,exam_arg->startwire),end,start,end);
    exam_arg->found = TRUE;
    exam_arg->startwire = cfgarg->startwire;
    return;
  }

  if ( (cfgdata & bitdata) == cfgdata ) {
    g_warning("Spurious bits for %s -> %s, config %i != bitdata %i",
	      start,end,cfgdata,bitdata);
    if (exam_arg->found)
      g_warning("Not replacing %s -> %s",
		wire_name(wiredb,exam_arg->startwire),end);
    else {
      exam_arg->found = TRUE;
      exam_arg->startwire = cfgarg->startwire;
    }
  }
}

typedef struct _examine_endpoint_memory {
  const bitstream_parsed_t *bitstream;
  const csite_descr_t *site;
  /* at some point get rid of wiredb */
  wire_db_t *wiredb;
  GArray *array;
} examine_endpoint_memory_t;

static void examine_groupnode (GNode *node,
			       gpointer data) {
  localpip_control_data_t *ctrldat = node->data;
  examine_endpoint_memory_t *exam_arg = data;
  wire_db_t * wiredb = exam_arg->wiredb;
  const wire_atom_t endwire = ctrldat->endwire;
  examine_data_memory_t pass_arg = {
    .endwire = endwire,
    .found = FALSE,
    .wiredb = wiredb,
  };
  guint32 bitdata;

  /* query the bitstream about the endpoint */
  bitdata = query_bitstream_site_bits(exam_arg->bitstream, exam_arg->site,
				      ctrldat->data, ctrldat->size);

  if (bitdata == 0)
    return;

  pass_arg.bitstream_data = bitdata;

  /* iterate over pips to find out who is okay. This could just be an array. */
  g_node_children_foreach(node,G_TRAVERSE_ALL,examine_node_memory,&pass_arg);

  if (pass_arg.found) {
    pip_t pip = { .source = pass_arg.startwire, .target = endwire };
    GArray *array = exam_arg->array;
    g_array_append_val(array, pip);
  }
}

/** \brief Query a bitstream for the pips contained in a site, in-memory version
 *
 * This is a very raw unoptimized version which should be must faster already
 *
 * @param pipdb the pip database
 * @param bitstream the bitstream data
 * @param site the site queried
 * @param size pointer to return the size of the db
 *
 * @return the list of pips which are present at the location
 * @return the length of this pip list
 */
pip_t *__pips_of_site_memory(const pip_db_t *pipdb,
			     const bitstream_parsed_t *bitstream,
			     const csite_descr_t *site,
			     gsize *size) {
  GArray *pips_array = g_array_new(FALSE, FALSE, sizeof(pip_t));
  GNode *head = pipdb->memorydb[site->type];
  examine_endpoint_memory_t exam_arg = {
    .bitstream = bitstream,
    .site = site,
    .array = pips_array,
    .wiredb = pipdb->wiredb,
  };

  iterate_over_groups_memory(head, examine_groupnode, &exam_arg);

  *size = pips_array->len;
  return (pip_t *) g_array_free (pips_array, FALSE);
}

pip_t *pips_of_site(const pip_db_t *pipdb,
		    const bitstream_parsed_t *bitstream,
		    const csite_descr_t *site,
		    gsize *size) {
  if (!pipdb->memorydb[site->type]) {
    *size = 0;
    return NULL;
  }
  return __pips_of_site_memory(pipdb, bitstream, site, size);
}

/*
 * Iterate over bitstreams in the bitstream
 * For now this duplicates a lot of code.
 * Code sharing with pip dumping should be restored at some point.
 */

typedef struct _examine_bitstream_memory {
  guint32 bitstream_data;
  wire_atom_t startwire;
  gboolean found;
} examine_bitstream_memory_t;

static void
examine_bit_node_memory (GNode *node, gpointer data) {
  examine_bitstream_memory_t *exam_arg = data;
  localpip_data_t *cfgarg = node->data;
  const guint32 cfgdata = cfgarg->cfgdata;
  const guint32 bitdata = exam_arg->bitstream_data;

  /* Please keep these warnings on, they indicate debitting
     idiosyncrasies that we don't fully get yet */
  if ( bitdata == cfgdata ) {
    exam_arg->found = TRUE;
    exam_arg->startwire = cfgarg->startwire;
    return;
  }

  if ( (cfgdata & bitdata) == cfgdata ) {
    if (!exam_arg->found) {
      exam_arg->found = TRUE;
      exam_arg->startwire = cfgarg->startwire;
    }
  }
}

typedef struct _examine_endpoint_bit_memory {
  const bitstream_parsed_t *bitstream;
  const site_ref_t site;
  bitpip_iterator_t func;
  gpointer data;
} examine_endpoint_bit_memory_t;

static void
examine_bit_groupnode (GNode *node, gpointer data) {
  localpip_control_data_t *ctrldat = node->data;
  examine_endpoint_bit_memory_t *exam_arg = data;
  const wire_atom_t endwire = ctrldat->endwire;
  examine_bitstream_memory_t pass_arg = {
    .found = FALSE,
  };
  guint32 bitdata;

  /* query the bitstream about the endpoint */
  bitdata = query_bitstream_site_bits(exam_arg->bitstream, exam_arg->site,
				      ctrldat->data, ctrldat->size);

  if (bitdata == 0)
    return;

  pass_arg.bitstream_data = bitdata;

  /* iterate over pips to find out who is okay. This could just be an array. */
  g_node_children_foreach(node,G_TRAVERSE_ALL, examine_bit_node_memory, &pass_arg);

  if (pass_arg.found)
    exam_arg->func(data, pass_arg.startwire, endwire, exam_arg->site);
}

/** \brief Iterator over pips in the bitstream
 */

void
iterate_over_bitpips(const pip_db_t *pipdb,
		     const bitstream_parsed_t *bitstream,
		     const site_ref_t site,
		     bitpip_iterator_t fun,
		     gpointer data) {

  GNode *head = pipdb->memorydb[site->type];
  examine_endpoint_bit_memory_t exam_arg = {
    .bitstream = bitstream,
    .site = site,
    .data = data, .func = fun,
  };

  iterate_over_groups_memory(head, examine_bit_groupnode, &exam_arg);
}

/** \brief Query the wiring database to get the copper startpoint of
 * the orig wire
 *
 * Ask the wiring database so as to get back to the wire startpoint
 *
 * @param pipdb the pip database
 * @param wire the wire to fill in
 * @param orig the source wire
 *
 * @return error return
 */

int get_wire_startpoint(const pip_db_t *pipdb,
			const chip_descr_t *chipdb,
			sited_wire_t *wire,
			const sited_wire_t *orig) {
  wire_simple_t *wo = &pipdb->wiredb->wires[orig->wire];
  wire_atom_t ep = wo->ep;
  site_ref_t site = orig->site, ep_site;

  g_print("getting startpoint of wire %s\n",
	  wire_name(pipdb->wiredb, orig->wire));

  ep_site = translate_global_site(chipdb, site, wo->dx, wo->dy);
  if ((ep == 0) || (ep_site == NULL))
    return -1;

  wire->wire = ep;
  wire->site = ep_site;
  return 0;
}

typedef struct _find_endpoint_memory {
  const bitstream_parsed_t *bitstream;
  const csite_descr_t *site;
  const wire_atom_t endpoint;
  wire_db_t *wiredb;
  gboolean found;
  wire_atom_t startpoint;
} find_endpoint_memory_t;

/** \brief Query the wiring database to get the coopper startpoint of
 * the orig wire
 *
 * Ask the wiring database so as to get back to the wire startpoint
 *
 * @param pipdb the pip database
 * @param wire the wire to fill in
 * @param orig the source wire
 *
 * @return error return
 */

static void find_groupnode (GNode *node,
			    gpointer data) {
  localpip_control_data_t *ctrldat = node->data;
  find_endpoint_memory_t *find_arg = data;
  wire_db_t * wiredb = find_arg->wiredb;
  const wire_atom_t endwire = ctrldat->endwire;
  examine_data_memory_t pass_arg = {
    .endwire = endwire,
    .found = FALSE,
    .wiredb = wiredb,
  };
  guint32 bitdata;

  if (endwire == find_arg->endpoint)
    return;

  /* query the bitstream about the endpoint */
  bitdata = query_bitstream_site_bits(find_arg->bitstream, find_arg->site,
				      ctrldat->data, ctrldat->size);
  pass_arg.bitstream_data = bitdata;

  /* iterate over pips to find out who is okay. This could just be an array. */
  g_node_children_foreach(node,G_TRAVERSE_ALL,examine_node_memory,&pass_arg);

  find_arg->startpoint = pass_arg.startwire;
  find_arg->found = pass_arg.found;
}

gboolean
get_interconnect_startpoint(const pip_db_t *pipdb,
			    const bitstream_parsed_t *bitstream,
			    sited_wire_t *wire,
			    const sited_wire_t *orig) {
  GNode *head = pipdb->memorydb[orig->site->type];
  find_endpoint_memory_t find_arg = {
    .bitstream = bitstream,
    .endpoint = orig->wire,
    .site = orig->site,
    .wiredb = pipdb->wiredb,
  };

  /* Iterate to find the node containing the origin */
  iterate_over_groups_memory(head, find_groupnode, &find_arg);

  if (find_arg.found) {
    wire->site = orig->site;
    wire->wire = find_arg.startpoint;
    return TRUE;
  }

  return FALSE;
}
