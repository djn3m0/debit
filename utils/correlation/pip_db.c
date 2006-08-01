/*
 * (C) Copyright 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
 * All rights reserved.
 *
 */

#include <glib.h>
#include "bitisolation_db.h"

const state_t *
get_pip_state(const pip_db_t *pipdb, const unsigned i) {
  return &pipdb->pip_array[i].state;
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
  return GPOINTER_TO_UINT(g_hash_table_lookup (pipdb->hash, pip));
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
  gpointer orig_key, value;

  chunk = g_string_chunk_insert_const (db->chunk, line);

  /* query & insert it in the LUT */
  if (g_hash_table_lookup_extended (hash, chunk, &orig_key, &value))
    return;

  g_hash_table_insert (hash, (gpointer)chunk, GUINT_TO_POINTER(g_hash_table_size(hash)));
}

void
iterate_over_lines(const gchar *filename,
		   line_iterator_t iter, void *data) {
  pip_db_t *pipdb = data;
  gchar *contents;
  gchar **lines;

  g_file_get_contents(filename, &contents, NULL, NULL);
  lines = g_strsplit(contents, "\n", 0);
  g_free(contents);

  while(*lines != NULL)
    iter(*lines++, pipdb);

  g_strfreev(lines);
}

static void
add_pip_file(const gchar *filename, void *data) {
  pip_db_t *pipdb = data;
  iterate_over_lines(filename, add_pip_line, pipdb);
}

static void
store_iline(gpointer key,
	    gpointer value,
	    gpointer user_data) {
  guint index = GPOINTER_TO_UINT(value);
  pip_db_t *db = user_data;
  pip_ref_t *ref = &db->pip_array[index];

  ref->start = key;
  ref->end = key;
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
  g_free(db);
}

/* FIXME: move this to one big allocation array */
int
alloc_pips_state(const pip_db_t *pip_db,
		 const size_t len, const size_t ulen) {
  unsigned npips = pip_db->pip_num;
  pip_ref_t *piparray = pip_db->pip_array;
  unsigned i;
  /* allocate state memory */
  for(i = 0; i < npips; i++) {
    state_t *state = &piparray[i].state;
    alloc_state(state, len, ulen);
    init_state(state, len, ulen);
  }
  return 0;
}

void free_pips_state(const pip_db_t *pip_db) {
  unsigned npips = pip_db->pip_num;
  pip_ref_t *piparray = pip_db->pip_array;
  unsigned i;
  for(i = 0; i < npips; i++) {
    state_t *alloced = &piparray[i].state;
    release_state(alloced);
  }
}
