/*
 * (C) Copyright 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
 * All rights reserved.
 *
 */

/*
 * include file common to pip_db.c and state_db.c
 */

#ifndef _HAS_BITISOLATION_H
#define _HAS_BITISOLATION_H

#include "bitisolation.h"

/* Ugly and infamous constants */
#define MAXR 100
#define MAXC 100
//#define n_pips 2827
#define n_pips 2847

typedef struct alldata {
  size_t known_data_len;
  size_t unknown_data_len;
  size_t nstates;
  state_t *states;
} alldata_t;

/*
 * Currently-used API
 */

/* PIP database */
typedef struct pip_ref {
  char *start;
  char *end;
  int isolated;
  /* The bitdata corresponding to the pip */
  state_t state;
} pip_ref_t;

typedef struct _pip_db {
  GHashTable *hash;
  GStringChunk *chunk;
  /* well, no need for these any more now */
  unsigned pip_num;
  pip_ref_t *pip_array;
} pip_db_t;

/* raw data database -- (xdl data, bitstream) site data pairs */
alldata_t *fill_all_data(const pip_db_t *, const gchar **, const gchar **);
void free_all_data(alldata_t *);

typedef void (*line_iterator_t)(const gchar *line, void *data);

void
iterate_over_lines(const gchar *filename,
		   line_iterator_t iter, void *data);

/* iterator over the db */
typedef void (*pip_iterator_t)(pip_ref_t *, void *);
void iterate_over_pips(const pip_db_t *, pip_iterator_t, void *);

pip_db_t *build_pip_db(const gchar **);
void free_pip_db(pip_db_t *);

void alloc_pips_state(pip_db_t *pipdb, const size_t len, const size_t ulen);
void free_pips_state(pip_db_t *pipdb);

const char *get_pip_start(const pip_db_t *pipdb, const unsigned i);
const char *get_pip_end(const pip_db_t *pipdb, const unsigned i);
const state_t *get_pip_state(const pip_db_t *pipdb, const unsigned i);
pip_ref_t  *get_pip(const pip_db_t *pipdb, const unsigned i);
unsigned get_pip_index(const pip_db_t *pipdb, const gchar *pip);

#endif /* _HAS_BITISOLATION_H */
