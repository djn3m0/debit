/*
 * (C) Copyright 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
 * All rights reserved.
 */

/*
 * New, simple wiring db implementation
 */

#include <stdio.h>
#include <glib.h>
#include <string.h>
#include "debitlog.h"

#include "wiring.h"
#include "design.h"

#ifdef __COMPILED_WIREDB

/*
 * Using a compiled DB is really a breeze.
 */

#if defined(VIRTEX2)
#define WIREDB "data/virtex2/wires.m4"
#elif defined(VIRTEX4)
#define WIREDB "data/virtex4/wires.m4"
#elif defined(VIRTEX5)
#define WIREDB "data/virtex5/wires.m4"
#elif defined(SPARTAN3)
#define WIREDB "data/spartan3/wires.m4"
#else
#error "Unable to compile wiredb in"
#endif

#include "data/wiring_compiled.h"

wire_db_t *get_wiredb(const gchar *datadir) {
  wire_db_t *wiredb = g_new0(wire_db_t, 1);
  (void) datadir;
  wiredb->details = details;
  wiredb->wires = wires;
  wiredb->wirenames = wirestr.str;
  wiredb->wireidx = wireidx;
  wiredb->dblen = G_N_ELEMENTS(wires);
  return wiredb;
}

void free_wiredb(wire_db_t *wires) {
  g_free(wires);
}

#else /* __COMPILED_WIREDB */

#define STRINGCHUNK_DEFAULT_SIZE 16

static gint
read_wiredb(GKeyFile **fill, const gchar *filename) {
  GKeyFile *db;
  GError *error = NULL;

  debit_log(L_WIRES, "Loading data from %s", filename);

  db = g_key_file_new();
  if (!db)
    goto out_err;

  g_key_file_load_from_file(db,filename,G_KEY_FILE_NONE,&error);

  if (error != NULL) {
    g_warning("could not read db %s: %s",filename,error->message);
    g_error_free (error);
    goto out_err_free;
  }

  *fill = db;
  return 0;

 out_err_free:
  g_key_file_free (db);
 out_err:
  return -1;
}

/*
 * Allocate a wire db
 */

static inline int
load_wire_atom(const wire_db_t *db, GKeyFile *keyfile,
	       const gchar *wirename)
{
  GError *err = NULL;
  int retval = 0;
  gint id = g_key_file_get_integer(keyfile, wirename, "ID", &err);
  wire_simple_t *wire = (void *) &db->wires[id];
  wire_t *detail = (void *) &db->details[id];

  if (err)
    goto out_err;

  /* Insert the wirename */
  debit_log(L_WIRES, "Inserting wire %s, id %i\n", wirename, id);
  db->names[id] = g_string_chunk_insert_const(db->wirenames, wirename);

#define GET_STRUCT_MEMBER(structname, structmem, strname) \
do { structname->structmem = g_key_file_get_integer(keyfile, wirename, #strname, &err);\
  if (err)\
    goto out_err;\
} while (0)

#define GET_STRUCT_MEMBER_LIST(structname, structmem, structmemlen, strname) \
do { structname->structmem = g_key_file_get_integer_list(keyfile, wirename, #strname, \
                                                         &structname->structmemlen, &err);\
  if (err)\
    goto out_err;\
} while (0)

  GET_STRUCT_MEMBER(wire, dx, DX);
  GET_STRUCT_MEMBER(wire, dy, DY);
  GET_STRUCT_MEMBER(wire, ep, EP);
  GET_STRUCT_MEMBER_LIST(wire, fut, fut_len, FUT);
  GET_STRUCT_MEMBER(detail, type, TYPE);
  GET_STRUCT_MEMBER(detail, direction, DIR);
  GET_STRUCT_MEMBER(detail, situation, SIT);

  return retval;

 out_err:
  g_warning("%s", err->message);
  retval = err->code;
  g_error_free (err);
  return retval;
}

static inline int
fill_db_from_file(const wire_db_t *wires, GKeyFile *db,
		  const gsize nwires, gchar **wirenames) {
  gint err = 0;
  gsize i;

  for(i = 0; i < nwires; i++) {
    err = load_wire_atom(wires, db, wirenames[i]);
    if (err)
      break;
  }

  return err;
}

/** Fill in a wire db with data from a file
 *
 * This function load from the db argument the wires argument
 *
 * @param db KeyFile containing data
 * @param wires wire database to fill
 * @return an error indication
 */
static int
load_db_from_file(GKeyFile* db, wire_db_t *wires) {
  gint err;
  gsize nwires;
  gchar** wirenames;

  wirenames = g_key_file_get_groups(db, &nwires);

  debit_log(L_WIRES, "Wiring database contains %zd wires\n", nwires);

  /* Allocate the array */
  wires->dblen = nwires;
  wires->wires = g_new(wire_simple_t, nwires);
  wires->names = g_new(const gchar *, nwires);
  wires->details = g_new(wire_t, nwires);
  wires->wirenames = g_string_chunk_new(STRINGCHUNK_DEFAULT_SIZE);

  /* Iterate over groups */
  err = fill_db_from_file(wires, db, nwires, wirenames);

  /* Cleanup */
  g_strfreev(wirenames);

  return err;
}


/*
 * High-level function
 */
wire_db_t *get_wiredb(const gchar *datadir) {
  wire_db_t *wiredb = g_new0(wire_db_t, 1);
  GKeyFile *db = NULL;
  gchar *dbname;
  gint err;

  dbname = g_build_filename(datadir,CHIP,"wires.db",NULL);

  err = read_wiredb(&db, dbname);
  g_free(dbname);
  if (err)
    goto out_err;

  err = load_db_from_file(db, wiredb);
  if (err)
    goto out_err;

  g_key_file_free(db);
  return wiredb;

 out_err:
  g_warning("failed to readback wire db");
  if (db)
    g_key_file_free(db);
  g_free(wiredb);
  return NULL;
}


/*
 * Free a wire db
 */

void free_wiredb(wire_db_t *wires) {
  GStringChunk *wirenames = wires->wirenames;
  if (wirenames)
    g_string_chunk_free(wirenames);
  g_free((void *)wires->details);
  g_free(wires->names);
  g_free((void *)wires->wires);
  g_free(wires);
}

#endif /* __COMPILED_WIREDB */

/*
 * Interface functions needed by wiring.h
 */

static inline gint cmp(const gchar *s1, const gchar *s2) {
  return strcmp(s1,s2);
}

/* This one is a get_by_name. The only complicated function, which
   is a dichotomy */

gint parse_wire_simple(const wire_db_t *db, wire_atom_t* res,
		       const gchar *wire) {
  guint low = 0, high = db->dblen-1;
  //  g_assert(high <= db->dblen);

  do {
    guint middle = (high + low) / 2;
    const gchar *middlename = wire_name(db, middle);
    gint comp = cmp(wire,middlename);
    if (!comp) {
      *res = middle;
      return 0;
    }
    if (comp > 0)
      low = middle + 1;
    else
      high = middle - 1;
  } while(low <= high);

  return -1;
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

gboolean
get_wire_startpoint(const wire_db_t *wiredb,
		    const chip_descr_t *chipdb,
		    site_ref_t *starget,
		    wire_atom_t *wtarget,
		    const site_ref_t sorig,
		    const wire_atom_t worig) {
  const wire_simple_t *wo = &wiredb->wires[worig];
  wire_atom_t ep = wo->ep;
  unsigned dx = 0, dy = 0;
  site_ref_t ep_site;
  int ret;

  debit_log(L_WIRES, "getting startpoint of wire %s\n",
	    wire_name(wiredb, worig));

  /* This is how we detect unknown wires in the db */
  if (ep == worig)
    return FALSE;

  ret = translate_global_site(chipdb, sorig, -wo->dx, -wo->dy,
			      &ep_site, &dx, &dy);
  if (ret) {
    if (wiredb->wires[ep].fut) {
      wire_atom_t target = wiredb->wires[ep].fut[dy];
      char coucou[32];
      sprint_switch(coucou, chipdb, ep_site);
      g_warning("patching %s to %s at site %s\n",
		wire_name(wiredb, worig),
		wire_name(wiredb, target),
		coucou);
      *wtarget = target;
      *starget = ep_site;
      return TRUE;
    }
    return FALSE;
  }

  *wtarget = ep;
  *starget = ep_site;
  return TRUE;
}

/* \brief Print a sited_pip_t
 *
 * @param buf the string buffer where to do the print
 * @param wdb the wire database
 * @param spip the sited pip to print
 */

void
sprint_spip(gchar *buf,
	    const wire_db_t *wdb,
	    const chip_descr_t *chip,
	    const sited_pip_t *spip) {
  const gchar *start = wire_name(wdb, spip->pip.source);
  const gchar *end = wire_name(wdb, spip->pip.target);
  gchar site_buf[30];

  sprint_switch(site_buf, chip, spip->site);
  sprintf(buf, "pip %s %s -> %s", site_buf, start, end);
}

