/*
 * (C) Copyright 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
 * All rights reserved.
 *
 */

#include <glib.h>
#include <string.h>
#include "debitlog.h"
#include "bitisolation_db.h"

const state_t *
get_pip_state(const pip_db_t *pipdb, const unsigned i) {
  return &pipdb->pip_array[i].state;
}

const char *
get_pip_name(const pip_db_t *pipdb, const unsigned i) {
  return pipdb->pip_array[i].name;
}

const char *
get_pip_start(const pip_db_t *pipdb, const unsigned i) {
  return pipdb->pip_array[i].start;
}

const char *
get_pip_end(const pip_db_t *pipdb, const unsigned i) {
  return pipdb->pip_array[i].end;
}

pip_ref_t *
get_pip(const pip_db_t *pipdb, const unsigned i) {
  return &pipdb->pip_array[i];
}

unsigned
get_pip_index(const pip_db_t *pipdb, const gchar *pip) {
  const gchar *chunk = g_string_chunk_insert_const (pipdb->chunk, pip);
  return GPOINTER_TO_UINT(g_hash_table_lookup (pipdb->hash, chunk));
}

typedef void (*ifile_iterator_t)(const gchar*, void *data);

void
iterate_over_input(const gchar **knw,
		   ifile_iterator_t iter, void *data) {
  unsigned idx = 0;
  const gchar *inp = knw[0];

  while (inp != NULL) {
    iter(inp, data);
    inp = knw[++idx];
  }
}

static void
add_pip_line(const gchar *line, void *data) {
  pip_db_t *db = data;
  const gchar *chunk;
  GHashTable *hash = db->hash;
  guint value_int;
  gpointer orig_key, value_ptr;

  chunk = g_string_chunk_insert_const (db->chunk, line);

  /* query & insert it in the LUT */
  if (g_hash_table_lookup_extended (hash, chunk, &orig_key, &value_ptr))
    return;
  value_int = g_hash_table_size(hash);
  value_ptr = GUINT_TO_POINTER(value_int);
  debit_log(L_CORRELATE, "Adding pip %s with value %u to the hashtable", chunk, value_int);
  g_hash_table_insert (hash, (gpointer)chunk, value_ptr);
}

void
iterate_over_lines(const gchar *filename,
		   line_iterator_t iter, void *data) {
  pip_db_t *pipdb = data;
  gchar *contents;
  gchar **lines, *line;
  unsigned i = 0;

  /* XXX handle failure, ungracefully */
  g_file_get_contents(filename, &contents, NULL, NULL);
  if (!contents) {
    g_warning("file %s does not exist!", filename);
    return;
  }

  lines = g_strsplit(contents, "\n", 0);
  g_free(contents);

  while((line = lines[i++]) != NULL)
    if (strlen(line) != 0)
      iter(line, pipdb);

  g_strfreev(lines);
}

static void
add_pip_file(const gchar *file, void *data) {
  pip_db_t *pipdb = data;
  gchar *filename = g_strconcat(file,".dat",NULL);
  debit_log(L_CORRELATE, "Loading file %s", filename);
  iterate_over_lines(filename, add_pip_line, pipdb);
  g_free(filename);
}

static void
store_iline(gpointer key,
	    gpointer value,
	    gpointer user_data) {
  guint index = GPOINTER_TO_UINT(value);
  pip_db_t *db = user_data;
  pip_ref_t *ref = &db->pip_array[index];
  gchar **endpoints = g_strsplit (key, " ", 2);

  ref->name = key;

  if (endpoints) {
    ref->start = endpoints[0];
    ref->end = endpoints[1];
  } else {
    ref->start = NULL;
    ref->end = NULL;
  }
  /* Don't free the strings themselves */
  g_free(endpoints);
}

/* build the pip db from a series of txt files */
pip_db_t *
build_pip_db(const gchar **files) {
  pip_db_t *db = g_new0(pip_db_t, 1);
  /* direct hash & compare thanks to the K-K chunky below */
  GHashTable *hash = g_hash_table_new (NULL, NULL);
  unsigned pipnum;

  db->chunk = g_string_chunk_new (16);
  db->hash = hash;

  iterate_over_input(files, add_pip_file, db);

  /* the summary of the table, nicely put */
  pipnum = g_hash_table_size(db->hash);
  db->pip_num = pipnum;
  db->pip_array = g_new(pip_ref_t, pipnum);
  g_hash_table_foreach (hash, store_iline, db);
  return db;
}

void free_pip_db(pip_db_t *db) {
  /* free states */
  g_hash_table_destroy(db->hash);
  g_string_chunk_free(db->chunk);
  g_free(db->pip_array);
  g_free(db);
}

void
iterate_over_pips(const pip_db_t *pipdb, pip_iterator_t iter, void *dat) {
  pip_ref_t *piparray = pipdb->pip_array;
  unsigned npips = pipdb->pip_num;
  unsigned pip;
  for(pip = 0; pip < npips; pip++) {
    pip_ref_t *pipref = &piparray[pip];
    iter(pipref, dat);
  }
}

typedef struct _both {
  unsigned len;
  unsigned ulen;
} both_t;

static void do_state(pip_ref_t *ref, void *dat) {
  both_t *arg = dat;
  unsigned len = arg->len, ulen = arg->ulen;
  state_t *state = &ref->state;
  alloc_state(state, len, ulen);
  init_state(state, len, ulen);
}

/* FIXME: move this to one big allocation array */
void
alloc_pips_state(pip_db_t *pip_db,
		 const size_t len, const size_t ulen) {
  both_t arg = { .len = len, .ulen = ulen };

  iterate_over_pips(pip_db, do_state, &arg);
}

static void free_state(pip_ref_t *ref, void *dat) {
  (void) dat;
  release_state(&ref->state);
}

void free_pips_state(pip_db_t *pipdb) {
  iterate_over_pips(pipdb, free_state, NULL);
}
