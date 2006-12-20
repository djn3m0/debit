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

#include <glib.h>
#include "debitlog.h"

#include "keyfile.h"

#include "bitstream.h"
#include "localpips.h"
#include "wiring.h"

#include "cfgbit.h"

#define STRINGCHUNK_DEFAULT_SIZE 16

/*
 * Forward declarations
 */

static int build_datatree_from_keyfiles(GKeyFile *data, GKeyFile *control,
					wire_db_t *wires, GNode *head,
					const site_type_t type);
static void destroy_datatree(GNode *head);

static const gchar *basedbnames[NR_SITE_TYPE] = {
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

  for (i = 0; i < NR_SITE_TYPE; i++) {
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
				       pipdb->memorydb[i], i);
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

  for(i = 0; i < NR_SITE_TYPE; i++) {
    GNode *tree = pipdb->memorydb[i];
    if (tree)
      destroy_datatree(tree);
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
  const site_type_t type;
} build_groupnode_t;

static inline gint *get_pip_structure_from_file(GKeyFile *keyfile,
						const gchar *end,
						gsize *length,
						const site_type_t type);

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

  dat->data = (guint *)get_pip_structure_from_file(exam->ctrldb, endp, &dat->size, exam->type);

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
			     wire_db_t *wires, GNode *head,
			     const site_type_t i) {
  build_groupnode_t arg = {
    .ctrldb = control,
    .head = head,
    .wires = wires,
    .type = i,
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
  g_print("pip %s -> %s", start, end);
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

/* XXX Do the transform at database-generation time */

static inline gint *get_pip_structure_from_file(GKeyFile *keyfile,
						const gchar *end,
						gsize *length, const site_type_t type) {
  gint *array = g_key_file_get_integer_list(keyfile, end, "BITLIST", length, NULL);
  const guint width = type_bits[type].y_width;
  gsize i;
  for (i = 0; i < *length; i++)
    array[i] = bitpos_to_cfgbit(array[i], width);
  return array;
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
    if (exam_arg->found) {
      debit_log(L_PIPS, "Perfect match replacing %s -> %s, with %s -> %s",
		wire_name(wiredb,exam_arg->startwire),end,start,end);
    }
    exam_arg->found = TRUE;
    exam_arg->startwire = cfgarg->startwire;
    return;
  }

  if ( (cfgdata & bitdata) == cfgdata ) {
    debit_log(L_PIPS, "Spurious bits for %s -> %s, config %i != bitdata %i",
	      start,end,cfgdata,bitdata);
    if (exam_arg->found) {
      debit_log(L_PIPS, "Not replacing %s -> %s",
		wire_name(wiredb,exam_arg->startwire),end);
    }
    else {
      exam_arg->found = TRUE;
      exam_arg->startwire = cfgarg->startwire;
    }
  }
  (void) start;
  (void) end;
}

typedef struct _examine_endpoint_memory {
  const bitstream_parsed_t *bitstream;
  const csite_descr_t *site;
  /* at some point get rid of wiredb */
  wire_db_t *wiredb;
  GArray *array;
} examine_endpoint_memory_t;

static void
examine_groupnode (GNode *node, gpointer data) {
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

static void
__pips_of_site_append(const pip_db_t *pipdb,
		      const bitstream_parsed_t *bitstream,
		      const csite_descr_t *site,
		      GArray *pips_array) {
  GNode *head = pipdb->memorydb[site->type];
  examine_endpoint_memory_t exam_arg = {
    .bitstream = bitstream,
    .site = site,
    .array = pips_array,
    .wiredb = pipdb->wiredb,
  };

  if (!pipdb->memorydb[site->type])
    return;

  iterate_over_groups_memory(head, examine_groupnode, &exam_arg);
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
static pip_t *
__pips_of_site_memory(const pip_db_t *pipdb,
		      const bitstream_parsed_t *bitstream,
		      const csite_descr_t *site,
		      gsize *size) {
  GArray *pips_array = g_array_new(FALSE, FALSE, sizeof(pip_t));
  __pips_of_site_append(pipdb, bitstream, site, pips_array);
  *size = pips_array->len;
  return (pip_t *) g_array_free (pips_array, FALSE);
}

pip_t *pips_of_site(const pip_db_t *pipdb,
		    const bitstream_parsed_t *bitstream,
		    const csite_descr_t *site,
		    gsize *size) {
  return __pips_of_site_memory(pipdb, bitstream, site, size);
}

/** \brief Do the bitstream pip interpretation in an ad-hoc, fast lookup
 * structure. This is the read/write mode, which is vaguely unoptimized,
 * but will allow for read-write operation. This is the filling
 * function, which takes an already-allocated structure.
 *
 * @param pipdb the pip database
 * @param chipdb the chip description
 * @param bitstream the bitstream data to read from
 *
 * @return a pip_parsed_t structure containing the bitstream structure
 */

typedef struct _allpips_iter {
  const bitstream_parsed_t *bitstream;
  const pip_db_t *pipdb;
  unsigned site_idx;
  unsigned *site_index;
  GArray *array;
} allpips_iter_t;

static void
_pips_of_bitstream_iter(unsigned site_x, unsigned site_y,
			csite_descr_t *site, gpointer dat) {
  allpips_iter_t *data = dat;
  unsigned *site_index = data->site_index;
  GArray *pips_array = data->array;

  (void) site_x;
  (void) site_y;

  /* Register the current pip index */
  site_index[data->site_idx++] = pips_array->len;
  /* Get back our pips */
  __pips_of_site_append(data->pipdb, data->bitstream, site, pips_array);
}

int
_pips_of_bitstream(const pip_db_t *pipdb, const chip_descr_t *chipdb,
		   const bitstream_parsed_t *bitstream,
		   pip_parsed_dense_t *fill) {
  /* This array will hold *all* of the pips */
  GArray *pips_array = g_array_new(FALSE, FALSE, sizeof(pip_t));
  gsize nsites = chipdb->width * chipdb->height;
  unsigned *site_index = g_new0(unsigned, nsites + 1);

  allpips_iter_t arg = {
    .bitstream = bitstream,
    .pipdb = pipdb,
    .array = pips_array,
    .site_idx = 0,
    .site_index = site_index,
  };

  iterate_over_sites(chipdb, _pips_of_bitstream_iter, &arg);

  site_index[nsites] = pips_array->len;
  debit_log(L_PIPS, "Got %i explicit pips", pips_array->len);

  fill->site_index = site_index;
  fill->bitpips = (pip_t *)g_array_free (pips_array, FALSE);

  return 0;
}

pip_parsed_dense_t *
pips_of_bitstream(const pip_db_t *pipdb, const chip_descr_t *chipdb,
		  const bitstream_parsed_t *bitstream) {
  pip_parsed_dense_t *dense = g_new(pip_parsed_dense_t, 1);
  int err;
  err = _pips_of_bitstream(pipdb, chipdb, bitstream, dense);
  if (err) {
    g_free(dense);
    return NULL;
  }
  return dense;
}

void free_pipdat(pip_parsed_dense_t *pipdat) {
  g_free(pipdat->site_index);
  g_free(pipdat->bitpips);
  g_free(pipdat);
}

/** \brief Query the pip database to get the origin of a pip
 *
 * This function guarantees that the spip won't be touched if
 * it returns FALSE. Other parts of the code rely on this !
 *
 * @param pipdb the pip database
 * @param wire the wire to fill in
 * @param orig the source wire
 *
 * @return if there's a pip driving the origin at the given site
 */

gboolean
get_interconnect_startpoint(const pip_db_t *pipdb,
			    const chip_descr_t *chip,
			    const pip_parsed_dense_t *pipdat,
			    wire_atom_t *wire,
			    const wire_atom_t orig,
			    const site_ref_t site) {
  unsigned stidx = site_index(chip, site);
  unsigned *indexes = pipdat->site_index;
  unsigned start = indexes[stidx], end = indexes[stidx+1];

  /* Do a run over the set of points for a site */
  while (start < end) {
    pip_t *pip = &pipdat->bitpips[start++];
    if (pip->target == orig) {
      *wire = pip->source;
      return TRUE;
    }
  }
  return FALSE;
}

pip_t *
pips_of_site_dense(const pip_parsed_dense_t *pipdat,
		   const chip_descr_t *chip,
		   const csite_descr_t *site,
		   gsize *size) {
  unsigned stidx = site_index(chip, site);
  unsigned *indexes = pipdat->site_index;
  unsigned start = indexes[stidx], end = indexes[stidx+1];

  *size = (end - start);
  return &pipdat->bitpips[start];
}

/** \brief Iterator over pips which are set in the bitstream
 */

void
iterate_over_bitpips(const pip_parsed_dense_t *pipdat,
		     const chip_descr_t *chip,
		     bitpip_iterator_t fun, gpointer data) {
  unsigned nsites = chip->width * chip->height;
  site_ref_t site = chip->data;
  unsigned *indexes = pipdat->site_index;
  unsigned start = 0, i;

  for (i = 0; i < nsites; i++) {
    unsigned end = indexes[i+1];
    for ( ; start < end; start++) {
      pip_t *pip = &pipdat->bitpips[start];
      debit_log(L_PIPS, "calling iterator for site %p", site);
      fun(data, pip->source, pip->target, site);
    }
    start = end;
    site++;
  }
}

/*
 * Complex iterator needed for optimal display.
 * A function is called on site change whose result is used to skip the
 * pip iteration (result is FALSE) or not (when result is TRUE).
 */

void
iterate_over_bitpips_complex(const pip_parsed_dense_t *pipdat,
			     const chip_descr_t *chip,
			     bitpip_iterator_t fun, gpointer data) {
  unsigned nsites = chip->width * chip->height;
  site_ref_t site = chip->data;
  unsigned *indexes = pipdat->site_index;
  unsigned start = 0, i;

  for (i = 0; i < nsites; i++) {
    unsigned end = indexes[i+1];
    for ( ; start < end; start++) {
      pip_t *pip = &pipdat->bitpips[start];
      debit_log(L_PIPS, "calling iterator for site %p", site);
      fun(data, pip->source, pip->target, site);
    }
    start = end;
    site++;
  }
}
