/*
 * (C) Copyright 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
 * All rights reserved.
 *
 */

#include <glib.h>
#include "bitisolation_db.h"

static int
mmap_fd_data(unsigned char **dst, const char *fname) {
  return -1;
}

/* VIRTEX-SPECIFIC CODE */
#define MAX_FNAME 64

static int
get_known_data(state_t *s, const unsigned row, const unsigned col) {
  /* First, the binary data generation is done in perl */
  char fname[MAX_FNAME];
  snprintf(fname,MAX_FNAME,"R%iC%i_known.bin",row,col);
  //fprintf(stderr, "mmapping %s @%p\n", fname, s);
  return mmap_fd_data(&s->known_data,fname);
}

static int
get_unknown_data(state_t *s, const unsigned row, const unsigned col) {
  char fname[MAX_FNAME];
  /* XXX factor this with other name-generating snipper in device.c */
  snprintf(fname,MAX_FNAME,"R%iC%i.bin",row,col);
  //fprintf(stderr, "mmapping %s @%p\n", fname, s);
  return mmap_fd_data(&s->unknown_data,fname);
}

typedef struct db_ref {
  unsigned row;
  unsigned col;
  /* bitfile */
  unsigned bitfile;
} db_ref_t;

static db_ref_t *db_array;

static void store_db(const unsigned idx,
		     const unsigned col,
		     const unsigned row) {
  db_array[idx].row = row;
  db_array[idx].col = col;
}

static void alloc_db(const unsigned nelems) {
  db_array = g_new0(db_ref_t, nelems);
}

static void free_db() {
  g_free(db_array);
}


void free_all_data() {
  /* FIXME: loop and munmap */
  free_db();
}

/* Iterate over all sites to get all data */
unsigned
fill_all_data(alldata_t *alldata) {
  /* XXX dumb, should not depend on R/C */
  int i,j;
  unsigned idx = 0;
  alloc_db(MAXR * MAXC);
  for(i = 0; i < MAXR; i++)
    for(j = 0; j < MAXC; j++) {
      state_t *s = &alldata->states[idx];
      if (get_known_data(s,i,j)) {
	//fprintf(stderr, "no known data for position (%03i,%03i)\n", i, j);
	continue;
      }
      else if (get_unknown_data(s,i,j)) {
	/* just warn */
	fprintf(stderr, "no unknown data for position (%03i,%03i)\n", i, j);
      } else {
	store_db(idx,i,j);
      }
      idx++;
    }
  return idx;
}

/* static void print_file(FILE *to, unsigned idx) { */
/*   fprintf(to, "R%iC%i", */
/* 	  db_array[idx].row, */
/* 	  db_array[idx].col); */
/* } */
