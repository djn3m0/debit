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

static unsigned isolated = 0;
static unsigned unisolated = 0;
static unsigned nil = 0;

/*
 * Dump of results in a log file
 */

static FILE *result_log = NULL;
static void
dump_to_log(const gchar *string) {
  if (result_log)
    fputs(string, result_log);
  else
    fputs(string, stdout);
}

static inline void
dump_state(alldata_t *dat, const state_t *state) {
  unsigned width = dat->width;
  if (width)
    bitarray_print2D (width, state->unknown_data);
  else
    bitarray_print (state->unknown_data);
}

static void
dump_result(alldata_t *dat, const gchar *name, const state_t *state) {
  GPrintFunc handler;
  handler = g_set_print_handler (dump_to_log);
  g_print("%s ", name);
  dump_state(dat, state);
  (void) g_set_print_handler (handler);
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
      debit_log(L_CORRELATE, "intersecting bit %i (%i bits set) with config %i (%i bits set)",
		bit, bitcount, i, bitarray_ones_count(config->unknown_data));
      /* Our bit is present in this config, so we and directly */
      and_state(state, config);
      bitcount = bitarray_ones_count(state->unknown_data);
      debit_log(L_CORRELATE, "Only %i bits remaining", bitcount);
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
  size_t len = 8 * dat->known_data_len;
  size_t ulen = 8 * dat->unknown_data_len;
  const gchar *pipname = get_pip_start(pipdb,bit);
  /* initial state. The printing should be specific and done outside of
     this pip-agnostic function */
  debit_log(L_CORRELATE, "doing pip #%08i, %s... ", bit, pipname);

  alloc_state(&state, len, ulen);
  init_state(&state, len, ulen);

  status = isolate_bit_core(&state, dat, bit);
  switch(status) {
  case STATUS_NOTALONE:
    unisolated++;
    debit_log(L_CORRELATE, "not alone");
    break;
  case STATUS_NIL:
    nil++;
    debit_log(L_CORRELATE, "nil reached!");
    break;
  default:
    debit_log(L_CORRELATE, "isolated");
    isolated++;
    dump_result(dat,pipname,&state);
  }

  /* check for and report collisions */
  /* check_collision(bit, &state, dat); */

  release_state(&state);
}

/* static void __attribute__((unused)) */
/* check_collision(const unsigned bit, const state_t *s, alldata_t *dat) { */
/*   int i; */
/*   size_t len, ulen; */
/*   unsigned bitleft; */
/*   state_t *configs = dat->states; */

/*   len = dat->known_data_len; */
/*   ulen = dat->unknown_data_len; */

/*   /\* loop over all isolated bits *\/ */
/*   for(bitleft = 0; bitleft < n_pips; bitleft++) */
/*     if (bit_is_present(bitleft, s)) */
/*       /\* see in states, which are also present in the unknown data *\/ */
/*       for(i = 0; i < dat->nstates; i++) */
/* 	if (!known_bit_is_present(bit, &configs[i]) && */
/* 	    bit_is_present(bitleft, &configs[i])) { */
/* /\* 	  fprintf(stderr, "bit %i is also present in state %i, ", *\/ */
/* /\* 		  bitleft, i); *\/ */
/* /\* 	  print_file(stderr, i); *\/ */
/* /\* 	  fprintf(stderr, "\n"); *\/ */
/* 	} */
/* } */

void
do_all_pips(const pip_db_t *pipdb, alldata_t *dat) {
  unsigned npips = pipdb->pip_num;
  unsigned pip;
  debit_log(L_CORRELATE, "Trying to isolate %i pips", npips);
  for(pip = 0; pip < npips; pip++)
    isolate_bit(pipdb, pip, dat);
}

void
do_filtered_pips(const pip_db_t *pipdb, alldata_t *dat,
		 const char *start, const char *end) {
  unsigned npips = pipdb->pip_num;
  unsigned pip;
  state_t union_state, work_state;
  core_status_t status;
  size_t len = pipdb->pip_num;
  size_t ulen = 8 * dat->unknown_data_len;

  /* allocated and zeroed */
  alloc_state(&union_state, len, ulen);
  alloc_state(&work_state, len, ulen);

  debit_log(L_CORRELATE, "working on pip %s -> %s", start, end);

  /* check if the pip is okay, if it is, isolated it, then
     or all the bits */
  for(pip = 0; pip < npips; pip++) {
    const char *pip_start = get_pip_start(pipdb, pip);
    const char *pip_end = get_pip_end(pipdb, pip);
    init_state(&work_state, len, ulen);
    if ((start && !strcmp(pip_start,start)) ||
	(end && !strcmp(pip_end,end))) {
      status = isolate_bit_core(&work_state, dat, pip);
      if (status == STATUS_ISOLATED)
	or_state(&union_state, &work_state);
    }
  }
//  dump_state(pip, dat, &union_state);
}
