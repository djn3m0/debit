/*
 * (C) Copyright 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
 * All rights reserved.
 *
 */

#include <glib.h>
#include "bitisolation_db.h"

#define MAX_PIPNAME 64
static pip_ref_t *pip_db;

static int
pips_db_alloc(unsigned npips) {
  pip_db = g_new0(pip_ref_t, npips);
  return 0;
}

void
free_pips_db(unsigned npips) {
  unsigned i;
  for(i = 0; i < npips; i++) {
    char *start = pip_db[i].start;
    char *end = pip_db[i].end;
    if (start)
      g_free(start);
    if (end)
      g_free(end);
  }
  g_free(pip_db);
  return;
}

const state_t *
get_pip_state(const unsigned i) {
  return &pip_db[i].state;
}

const char *
get_pip_start(const unsigned i) {
  return pip_db[i].start;
}

const char *
get_pip_end(const unsigned i) {
  return pip_db[i].end;
}

pip_ref_t *
get_pip(const unsigned i) {
  return &pip_db[i];
}

unsigned
read_pips_db(const char *fname) {
  FILE *fd;
  char pip_start[MAX_PIPNAME];
  char pip_end[MAX_PIPNAME];
  unsigned pip_num;
  unsigned idx = 0;
  unsigned npips = n_pips;

  fd = fopen(fname,"r");
  pips_db_alloc(npips);

  while(1) {
    int ret;
    ret = fscanf(fd,"%s %s %i\n", pip_start, pip_end, &pip_num);
    if (ret == EOF)
      break;
    /* store in DB */
    idx++;
    pip_db[pip_num].start = strdup(pip_start);
    pip_db[pip_num].end = strdup(pip_end);
  }

  fclose(fd);
  /* XXX return number of pips */
  return npips;
}

/* FIXME: move this to one big allocation array */

int
alloc_pips_state(const unsigned npips,
		 const size_t len, const size_t ulen) {
  unsigned i;
  /* allocate state memory */
  for(i = 0; i < npips; i++) {
    state_t *alloced = &pip_db[i].state;
    alloc_state(alloced, len, ulen);
    init_state(alloced, len, ulen);
  }
  return 0;
}

void free_pips_state(const unsigned npips) {
  unsigned i;
  for(i = 0; i < npips; i++) {
    state_t *alloced = &pip_db[i].state;
    release_state(alloced);
  }
}
