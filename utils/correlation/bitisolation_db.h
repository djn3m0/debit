/*
 * (C) Copyright 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
 * All rights reserved.
 *
 */

/*
 * include file common to pip_db.c and state_db.c
 */

#ifndef _HAS_BITISOLATION_DB_H
#define _HAS_BITISOLATION_DB_H

#include "bitisolation.h"

#define BYTES_OF_BITS(x) ((x + 7) / 8)

/*
 * Currently-used API
 */

typedef enum _pip_status {
  PIP_ACCOMPANIED,
  PIP_VOID,
  PIP_ISOLATED,
} pip_status_t;

/* PIP database */
typedef struct pip_ref {
  const char *name;
  char *start;
  char *end;
  pip_status_t isolated;
} pip_ref_t;

typedef struct _pip_db {
  GHashTable *hash;
  GStringChunk *chunk;
  /* well, no need for these any more now */
  unsigned pip_num;
  pip_ref_t *pip_array;
  state_t *state_array;
} pip_db_t;

/* raw data database -- (xdl data, bitstream) site data pairs */
alldata_t *fill_all_data(const pip_db_t *, const gchar *, const gchar **);
void free_all_data(alldata_t *);

typedef void (*line_iterator_t)(const gchar *line, void *data);

void
iterate_over_lines(const gchar *filename,
		   line_iterator_t iter, void *data);

/* iterator over the db */
typedef void (*pip_iterator_t)(pip_ref_t *, state_t *, void *);
void iterate_over_pips(const pip_db_t *, pip_iterator_t, void *);

pip_db_t *build_pip_db(const gchar **);
void free_pip_db(pip_db_t *);

void alloc_pips_state(pip_db_t *pip_db, const alldata_t *dat);
void free_pips_state(pip_db_t *pipdb);

const char *get_pip_name(const pip_db_t *pipdb, const unsigned i);
const char *get_pip_start(const pip_db_t *pipdb, const unsigned i);
const char *get_pip_end(const pip_db_t *pipdb, const unsigned i);
state_t *get_pip_state(const pip_db_t *pipdb, const unsigned i);
pip_ref_t  *get_pip(const pip_db_t *pipdb, const unsigned i);
unsigned get_pip_index(const pip_db_t *pipdb, const gchar *pip);
void dump_pips_db(const pip_db_t *pipdb);


#endif /* _HAS_BITISOLATION_DB_H */
