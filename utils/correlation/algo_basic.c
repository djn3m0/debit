/*
 * (C) Copyright 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
 * All rights reserved.
 *
 */

#include <stdio.h>
#include <glib.h>

#include "debitlog.h"

#include "bitisolation_db.h"
#include "bitisolation.h"
#include "algos.h"

/*
 * Dump of results in a log file
 */

static FILE *result_log = NULL;
static void
dump_to_log(const gchar *string) {
  if (result_log)
    fputs(string, result_log);
  else
    fputs(string, stderr);
}

static void
print_pip(const int bit, void *data) {
  const pip_db_t *pipdb = data;
  g_print("%s -> %s\n", get_pip_start(pipdb, bit), get_pip_end(pipdb, bit));
}

static inline void
dump_state(unsigned width, const state_t *state) {
  if (width)
    bitarray_print2D (width, state->unknown_data);
  else
    bitarray_print (state->unknown_data);
}

static void
dump_result(unsigned width, const state_t *state) {
  GPrintFunc handler;
  handler = g_set_print_handler (dump_to_log);
  dump_state(width, state);
  (void) g_set_print_handler (handler);
}

static inline void
dump_set(const state_t *state,
	 const pip_db_t *pipdb) {
  GPrintFunc handler;
  handler = g_set_print_handler (dump_to_log);
  g_print("Set of known bits ");
  bitarray_for_ones (state->known_data, print_pip, (void *)pipdb);
  (void) g_set_print_handler (handler);
}

/* Printing function */

typedef struct _db_stats_t {
  unsigned isolated;
  unsigned unisolated;
  unsigned nil;
} db_stats_t;

static void
print_stats(db_stats_t *stats, FILE *out) {
  fprintf(out, "DB has %u nil, %u isolated and %u unisolated pips",
	  stats->nil, stats->isolated, stats->unisolated);
}

typedef struct _print_db_t {
  const pip_db_t *pipdb;
  db_stats_t stats;
} print_db_t;

static void
print_pip_ref(pip_ref_t *ref, void *data) {
  print_db_t *dat = data;
  const pip_db_t *pipdb = dat->pipdb;
  fprintf(stderr,"pip %s -> %s:: ", ref->start, ref->end);

  switch (ref->isolated) {
  case PIP_VOID:
    dat->stats.nil++;
    fprintf(stderr,"NULL\n");
    break;
  case PIP_ACCOMPANIED:
    dat->stats.unisolated++;
    fprintf(stderr,"not alone\n");
    debit_log(L_CORRELATE, "together with:");
    dump_set(&ref->state, pipdb);
    debit_log(L_CORRELATE, "set bits:");
    dump_result(0,&ref->state);
    break;
  case PIP_ISOLATED:
    dat->stats.isolated++;
    fprintf(stderr,"isolated ");
    dump_result(0,&ref->state);
  }
}

void
dump_pips_db(const pip_db_t *pipdb) {
  print_db_t arg =
    {
      .pipdb = pipdb,
      .stats = {.isolated = 0, .unisolated = 0, .nil = 0 },
    };
  iterate_over_pips(pipdb, print_pip_ref, &arg);
  print_stats(&arg.stats, stderr);
}

/*
  First pass: do only intersections of state_db elements to get pip_db
  elements

  This function writes in the results array, which is a pip db, the
  intersection of all elements in origin which do contain the same pip
  as result.
*/
static void
intersect_same_pips(const state_t *result,
		    const size_t ndb, const state_t *db,
		    const unsigned bit) {
  unsigned i;
  /* loop over all available configuration */
  for(i = 0; i < ndb; i++) {
    const state_t *config = &db[i];
    if (known_bit_is_present(bit,config))
      and_state(result, config);
  }
}

/*
  Second pass: intersect pip_db elements with elements of state_db
  which are "compatible"

  This function writes in the results array, which is a pip db, the
  intersections of the negation of all elements in origin which are
  compatible with the same pip.
 */

/* typedef int  (*bitarray_iter_t)(const int,void *); */
typedef struct _ep_cmp {
  const pip_db_t *pipdb;
  const gchar *ep;
} ep_cmp_t;

static int
has_startpoint(const int pip, void *dat) {
  ep_cmp_t *epcmp = dat;
  /* We can compare-equal thanks to the stringchunk */
  return (get_pip_start(epcmp->pipdb, pip) == epcmp->ep);
}

static int
shares_startpoint(const pip_db_t *pipdb, const char *ep, const state_t *state) {
  /* For all pips present in the state, check whether
     the pip has the same endpoint or not */
  ep_cmp_t arg = { .pipdb = pipdb, .ep = ep };
  int ret = bitarray_iter_ones (state->known_data, has_startpoint, &arg);
  return ret;
}

static void
_intersect_compatible_pips(pip_ref_t *result,
			   const pip_db_t *pipdb,
			   const size_t ndb, const state_t *db,
			   const unsigned bit) {
  unsigned i;
  const char *endp = result->start;

  for(i = 0; i < ndb; i++) {
    const state_t *config = &db[i];
    if (!shares_startpoint(pipdb, endp, config))
      and_neg_state(&result->state, config);
  }
}

static void
gather_null_pips(state_t *result,
		 const pip_db_t *pipdb,
		 const alldata_t *dat) {
  /* Remove the null pips from the set of known data */
  unsigned i;

  init_state(result);

  for (i = 0; i < pipdb->pip_num; i++) {
    const pip_ref_t *pip = get_pip(pipdb, i);
    if (pip->isolated == PIP_VOID)
      bitarray_unset(result->known_data, i);
  }
}

void
prune_null_pips(const pip_db_t *pipdb,
		alldata_t *dat) {
  size_t len = dat->known_data_len;
  size_t ulen = dat->unknown_data_len;
  state_t state;
  unsigned i;

  alloc_state(&state, len, ulen);
  init_state(&state);

  gather_null_pips(&state, pipdb, dat);

  /* Then AND all states */
  for (i = 0; i < pipdb->pip_num; i++) {
    const state_t *pip = get_pip_state(pipdb,i);
    and_state(pip, &state);
  }

  for (i = 0; i < dat->nstates; i++)
    and_state(&dat->states[i], &state);

  release_state(&state);
}

/* Compute the status of a pip */
static pip_status_t
_flag_pip(const state_t *pip) {
  /* XXX count ones, and then... */
  if (unk_data_nil(pip))
    return PIP_VOID;

  if (is_isolated(pip))
    return PIP_ISOLATED;

  return PIP_ACCOMPANIED;
}

static inline void
flag_pip(pip_ref_t *pip) {
  pip->isolated = _flag_pip(&pip->state);
}

core_status_t
isolate_bit_core(const state_t *state,
		 const alldata_t *dat, const unsigned bit) {
  state_t *configs = dat->states;
  unsigned i;
  /* loop over all available configuration */
  for(i = 0; i < dat->nstates; i++) {
    state_t *config = &configs[i];
    /* fprintf(stderr,"Trying state %i\n",i); */
    /* Don't do anything if byte is not present, due to bit collision,
       for now unknown */
    if (known_bit_is_present(bit,config)) {
      unsigned bitcount = bitarray_ones_count(state->unknown_data);
/*       debit_log(L_CORRELATE, "intersecting bit %i (%i bits set) with config %i (%i bits set)", */
/* 		bit, bitcount, i, bitarray_ones_count(config->unknown_data)); */
      /* Our bit is present in this config, so we and directly */
      and_state(state, config);
      bitcount = bitarray_ones_count(state->unknown_data);
/*       debit_log(L_CORRELATE, "Only %i bits remaining", bitcount); */
    }
  }

  if (unk_data_nil(state))
    return STATUS_NIL;

  if (!is_isolated(state))
    return STATUS_NOTALONE;

  return STATUS_ISOLATED;
}

/* Other possibility -> dichotomy, and do a descent with take/don't
   contake. There's more data to memoize, but it should be far faster */
static void
isolate_bit(const pip_db_t *pipdb, const unsigned bit, alldata_t *dat) {
  state_t state;
  core_status_t status;
  size_t len = dat->known_data_len;
  size_t ulen = dat->unknown_data_len;
  const gchar *pipname = get_pip_name(pipdb,bit);

  /* initial state. The printing should be specific and done outside of
     this pip-agnostic function */
  debit_log(L_CORRELATE, "doing pip #%08i, %s... ", bit, pipname);

  alloc_state(&state, len, ulen);
  init_state(&state);

  status = isolate_bit_core(&state, dat, bit);
  switch(status) {
  case STATUS_NOTALONE:
    debit_log(L_CORRELATE, "not alone, together with:");
    dump_set(&state, pipdb);
    debit_log(L_CORRELATE, "set bits are:");
    dump_result(dat->width, &state);
    break;
  case STATUS_NIL:
    debit_log(L_CORRELATE, "nil reached!");
    break;
  default:
    debit_log(L_CORRELATE, "isolated");
    dump_result(dat->width, &state);
  }

  release_state(&state);
}

void
do_all_pips(const pip_db_t *pipdb, alldata_t *dat) {
  unsigned npips = pipdb->pip_num;
  unsigned pip;
  debit_log(L_CORRELATE, "Trying to isolate %i pips", npips);
  for(pip = 0; pip < npips; pip++)
    isolate_bit(pipdb, pip, dat);
}

static void
isolate_bit_thorough(const pip_db_t *pipdb,
		     const unsigned bit, alldata_t *dat) {
  state_t *db = dat->states;
  unsigned ndb = dat->nstates;
  pip_ref_t *pips = get_pip(pipdb, bit);

  debit_log(L_CORRELATE, "doing pip #%08i, %s... ", bit, pips->name);
  intersect_same_pips(&pips->state, ndb, db, bit);
  _intersect_compatible_pips(pips, pipdb, ndb, db, bit);
  flag_pip(pips);
}

void
do_all_pips_thorough(const pip_db_t *pipdb, alldata_t *dat) {
  unsigned npips = pipdb->pip_num;
  unsigned pip;
  debit_log(L_CORRELATE, "Trying to isolate %i pips", npips);

  for(pip = 0; pip < npips; pip++) {
    pip_ref_t *pipref = get_pip(pipdb, pip);
    isolate_bit_thorough(pipdb, pip, dat);
    flag_pip(pipref);
  }

  prune_null_pips(pipdb, dat);

  for(pip = 0; pip < npips; pip++) {
    pip_ref_t *pipref = get_pip(pipdb, pip);
    flag_pip(pipref);
  }

  /* Then print the db */
  dump_pips_db(pipdb);
}

void
do_filtered_pips(const pip_db_t *pipdb, alldata_t *dat,
		 const char *start, const char *end) {
  unsigned npips = pipdb->pip_num;
  unsigned pip;
  state_t union_state, work_state;
  core_status_t status;
  size_t len = dat->known_data_len;
  size_t ulen = dat->unknown_data_len;

  /* allocated and zeroed */
  alloc_state(&union_state, len, ulen);
  alloc_state(&work_state, len, ulen);

  debit_log(L_CORRELATE, "working on pip %s -> %s", start, end);

  /* check if the pip is okay, if it is, isolate it, then OR all the
     bits, so that we get the bit-set for a given endpoint. */
  for(pip = 0; pip < npips; pip++) {
    const char *pip_start = get_pip_start(pipdb, pip);
    const char *pip_end = get_pip_end(pipdb, pip);
    init_state(&work_state);
    if ((start && !strcmp(pip_start,start)) ||
	(end && !strcmp(pip_end,end))) {
      status = isolate_bit_core(&work_state, dat, pip);
      if (status == STATUS_ISOLATED)
	or_state(&union_state, &work_state);
    }
  }
//  dump_state(pip, dat, &union_state);

  release_state(&work_state);
  release_state(&union_state);
}

/*
 * TODO: understand theoretically if the substraction at the end of
 * individual things brings something to the table in the (negate if
 * filtered) case.
 *
 */
