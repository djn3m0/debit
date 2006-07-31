/*
 * (C) Copyright 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
 * All rights reserved.
 *
 */

/*
 * include file common to pip_db.c and state_db.c
 */

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
/* raw data database -- (xdl data, bitstream) site data pairs */
alldata_t *fill_all_data(const gchar **in, const gchar **out);
void free_all_data(alldata_t *);

/* PIP database */
typedef struct pip_ref {
  char *start;
  char *end;
  int isolated;
  /* The bitdata corresponding to the pip */
  state_t state;
} pip_ref_t;

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))

unsigned read_pips_db(const char *fname);
void free_pips_db(unsigned npips);

int alloc_pips_state(const unsigned npips, const size_t len, const size_t ulen);
void free_pips_state(const unsigned npips);

const char *get_pip_start(const unsigned i);
const char *get_pip_end(const unsigned i);
const state_t *get_pip_state(const unsigned i);
pip_ref_t  *get_pip(const unsigned i);

/*
 * Newer structures
 */

#include <db.h>

typedef struct upip_db {
  /* key is an int, data is the pip/configuration point name
     in the case of pips, the name is a string pair
  */
  DB pip_to_name;
  /* reverse-engeneering data
     - key is xdl, known data (for now generated from a file),
     - data is unknown (bitstream) configuration data
  */
  DB raw_data;

  /* key is an int, data is an array of ints, containing the associated
     configuration bits */
  DB pip_bits;

  /* typically xc2v2000 for now */
  const char *device;
  const char *directory;
} upip_db_t;

/* creates & resets the whole db */

/* load a new set of files into the raw_data db, typically done when
   inputting a new bit file. Ideally, this would be derived directly
   from the xdl and bit files, and pip_to_name would be automatically
   complemented. These databases would accumulate data automatically.
   In fact for now, you must be sure to have coherent data
   with the pip_to_known datafile. In short, update both the
   pip_to_known and raw_data at once. */
