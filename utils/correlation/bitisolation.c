/*
 * (C) Copyright 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
 * All rights reserved.
 *
 */

/* for strtol */
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
/* for memcmp */
#include <string.h>

#include "bitisolation_db.h"

/* Reverse-engeneering core.
   Try to do this data-agnostic. */

static void __attribute__((unused))
dump_known(FILE *out, const int bit,
	   const unsigned char *dat, const size_t len) {
  unsigned i;
  fprintf(out, "++dumping isolated state %i\n", bit);
  for (i=0; i < len; i++) {
    if (dat[i])
      fprintf(out, "--could not be separated from %i\n", i);
  }
}

static int
_is_isolated(FILE *out, const int bit,
	     const char *dat, const size_t len) {
  unsigned i;
  int ok = 0;
  for (i=0; i < len; i++) {
    if (dat[i]) {
      if (i != bit) {
	fprintf(out, "--could not be separated from %i\n", i);
	ok = -1;
      }
    }
  }
  return ok;
}

static void
dump_bin(FILE *out, const void *_data, const unsigned num) {
  const char *const data = _data;
  unsigned i;
  for( i = 0; i < num; i++) {
    char atom = data[i];
    putc(atom, out);
  }
}

static void
dump_bin_file(const char *fname,
	      const void *_data, const unsigned num) {
  FILE *f;
  f = fopen(fname, "w");
  dump_bin(f, _data, num);
  fclose(f);
}

static void
print_state(FILE *out,
	    const void *_data, const unsigned num) {
  unsigned i,j;
  const char *data = _data;
  for(i = 0; i < num; i++)
    for(j = 0; j < 8; j++)
      if (data[i] & (1 << j)) {
	unsigned offset = i*8 + j;
	unsigned xpos = 0;
	unsigned ypos = 0;
	fprintf(out, "%i (%i,%i)\n", offset, xpos, ypos);
      }
}

static void
dump_unknown(const unsigned bit, const char *dat, const size_t len) {
  char fn[64];
  snprintf(fn, 64, "isolated_%i.bin", bit);
  dump_bin_file(fn, dat, len);
  return;
}

static void
dump_state(const int bit, const alldata_t *dat, const state_t *state) {
  /* dump unknown data and commentary about the remaining known data */
  dump_unknown(bit, state->unknown_data, dat->unknown_data_len);
  print_state(stdout, state->unknown_data, dat->unknown_data_len);
}

static void
dump_pips_db(const pip_db_t *pipdb, const unsigned ulen) {
  const unsigned npips = pipdb->pip_num;
  int i;
  /* loop throught the whole db -- iterate_over_states */
  for (i = 0; i < npips; i++) {
    pip_ref_t *piprec = get_pip(pipdb, i);
    if (!piprec->isolated)
      continue;
    fprintf(stdout,"pip #%08i, %s -> %s\n",
	    i,get_pip_start(pipdb,i),get_pip_end(pipdb,i));
    print_state(stdout, piprec->state.unknown_data, ulen);
  }
}

static int __attribute__((unused))
cmpstringp(const void *p1, const void *p2)
{
  /* The arguments to this function are "pointers to
              pointers to char", but strcmp() arguments are "pointers
              to char", hence the following cast plus dereference */

  return strcmp(* (char **) p1, * (char **) p2);
}

/* static int cmpstatep(const void *p1, const void *p2) { */
/*   s1 = * (pip_ref_t **p1); */
/*   s2 = * (pip_ref_t **p2); */
/*   return strcmp(s1->end, s2->end); */
/* } */

/* static void */
/* dump_sorted_pips_db(const unsigned npips, const unsigned ulen) { */
/*   int i; */
/*   state_t *array[npips]; */
/*   /\* sort by output *\/ */
/*   qsort(array, npips, sizeof(state_t *), cmpstatep); */
/*   /\* loop throught the whole db *\/ */
/*   for (i = 0; i < npips; i++) { */
/*     pip_ref_t *piprec = get_pip(i); */
/*     if (!piprec->isolated) */
/*       continue; */
/*     fprintf(stdout,"pip #%08i, %s -> %s\n", */
/* 	    i,get_pip_start(i),get_pip_end(i)); */
/*     print_state(stdout, piprec->state.unknown_data, ulen); */
/*   } */
/* } */

/* TODO: move to uint32_t -- speed x4 */
static void
and_data(char *to, char *and, size_t len) {
  size_t i;
  for(i = 0; i < len; i++)
    to[i] = to[i] & and[i];
}

static void
or_data(char *to, char *or, size_t len) {
  size_t i;
  for(i = 0; i < len; i++)
    to[i] = to[i] | or[i];
}

static void
and_neg_data(char *to, char *and, size_t len) {
  size_t i;
  for(i = 0; i < len; i++)
    to[i] = to[i] & ~and[i];
}

static int
data_nil(char *data, size_t len) {
  size_t i;
  for(i = 0; i < len; i++)
    if (data[i] != 0)
      return data[i];
  return 0;
}

static void __attribute__((unused))
dump_bytes(FILE *out, const void *_data, int num)
{
  const char *const data = _data;
  const char *ptr = data;

  if (num <= 0) {
    fprintf(out, "\n");
    return;
  }

  while (num >= 16) {
    fprintf(out, "%04x:"
	    "%02X %02X %02X %02X %02X %02X %02X %02X "
	    "%02X %02X %02X %02X %02X %02X %02X %02X\n",
	    (short) (ptr - data),
	    ptr[0], ptr[1], ptr[2], ptr[3],
	    ptr[4], ptr[5], ptr[6], ptr[7],
	    ptr[8], ptr[9], ptr[10], ptr[11],
	    ptr[12], ptr[13], ptr[14], ptr[15]);
    num -= 16;
    ptr += 16;
  }
  if (num > 0) {
    fprintf(out, "%04x:", (short) (ptr - data));
    while (--num > 0)
      fprintf(out, "%02X ", *ptr++);
    fprintf(out, "%02X\n", *ptr);
  }
}

/* This function takes two states and ands them */
static void
and_state(const state_t *s1, const state_t *s2,
	  const size_t len, const size_t ulen) {
  and_data(s1->known_data, s2->known_data, len);
  and_data(s1->unknown_data, s2->unknown_data, ulen);
}

static void
or_state(const state_t *s1, const state_t *s2,
	 const size_t len, const size_t ulen) {
  or_data(s1->known_data, s2->known_data, len);
  or_data(s1->unknown_data, s2->unknown_data, ulen);
}

static void __attribute__((unused))
and_neg_state(state_t *s1, const state_t *s2,
	      size_t len, size_t ulen) {
  and_neg_data(s1->known_data, s2->known_data, len);
  and_neg_data(s1->unknown_data, s2->unknown_data, ulen);
}

static int
bit_is_present(const unsigned bit, const state_t *s) {
  /* bitarray lookup in known data */
  return s->unknown_data[bit / 8] & (1 << (bit % 8));
}

static int
byte_is_present(const unsigned byte, state_t *s) {
  /* bitarray lookup in known data */
  return s->known_data[byte];
}

static void __attribute__((unused))
check_collision(const unsigned bit, const state_t *s, alldata_t *dat);

static unsigned isolated = 0;
static unsigned unisolated = 0;
static unsigned nil = 0;

static void
print_summary(void) {
  fprintf(stdout,"Summary: isolated %i, nil reached %i, not isolated %i\n",
	  isolated, nil, unisolated);
}

typedef enum core_status {
  STATUS_NIL = 0,
  STATUS_NOTALONE,
  STATUS_ISOLATED,
} core_status_t;

static core_status_t
isolate_bit_core(const state_t *state,
		 const alldata_t *dat, const unsigned bit) {
  state_t *configs = dat->states;
  size_t len = dat->known_data_len;
  size_t ulen = dat->unknown_data_len;
  unsigned i;
  /* loop over all available configuration */
  for(i = 0; i < dat->nstates; i++) {
    /* fprintf(stderr,"Trying state %i\n",i); */
    /* Don't do anything if byte is not present, due to bit collision,
       for now unknown */
    if (byte_is_present(bit,&configs[i]))
      /* Our bit is present in this config, so we and directly */
      and_state(state, &configs[i], len, ulen);
  }

  if (!data_nil(state->unknown_data, ulen))
    return STATUS_NIL;

  if (_is_isolated(stderr,bit,state->known_data, len))
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

  /* initial state. The printing should be specific and done outside of
     this pip-agnostic function */
  fprintf(stdout,"doing pip #%08i, %s -> %s...",bit,
	  get_pip_start(pipdb,bit),get_pip_end(pipdb,bit));
  alloc_state(&state, len, ulen);
  init_state(&state, len, ulen);

  status = isolate_bit_core(&state, dat, bit);
  switch(status) {
  case STATUS_NOTALONE:
    unisolated++;
    //fprintf(stdout, "not alone\n");
    break;
  case STATUS_NIL:
    nil++;
    fprintf(stdout, "nil reached!\n");
    break;
  default:
    fprintf(stdout, "isolated\n");
    isolated++;
    dump_state(bit, dat, &state);
  }

  /* check for and report collisions */
  /* check_collision(bit, &state, dat); */

  release_state(&state);
}

static void __attribute__((unused))
check_collision(const unsigned bit, const state_t *s, alldata_t *dat) {
  int i;
  size_t len, ulen;
  unsigned bitleft;
  state_t *configs = dat->states;

  len = dat->known_data_len;
  ulen = dat->unknown_data_len;

  /* loop over all isolated bits */
  for(bitleft = 0; bitleft < n_pips; bitleft++)
    if (bit_is_present(bitleft, s))
      /* see in states, which are also present in the unknown data */
      for(i = 0; i < dat->nstates; i++)
	if (!byte_is_present(bit, &configs[i]) &&
	    bit_is_present(bitleft, &configs[i])) {
/* 	  fprintf(stderr, "bit %i is also present in state %i, ", */
/* 		  bitleft, i); */
/* 	  print_file(stderr, i); */
/* 	  fprintf(stderr, "\n"); */
	}
}

static void
do_all_pips(const pip_db_t *pipdb, alldata_t *dat) {
  unsigned npips = pipdb->pip_num;
  unsigned pip;
  for(pip = 0; pip < npips; pip++)
    isolate_bit(pipdb, pip, dat);
}

static void
do_filtered_pips(const pip_db_t *pipdb,
		 alldata_t *dat,
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

  fprintf(stdout, "working on pips %s -> %s", start, end);

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
	or_state(&union_state, &work_state, len, ulen);
    }
  }
  dump_state(pip, dat, &union_state);
}

/* Multi-pass algorithm
   First pass is standard allpips pass; it also flags and records pips
   according to a heuristic, so that some pips are "known-isolated".
   Second pass takes advantages of the flagged pips of the first pass to
   remove some configuration pips from non-isolated pips, hopefully
   leading to more isolated pips. Repeat until the non-isolated pips
   database doesn't grow anymore.
*/

/* This function counts the unknown bits and records their offset up to
   reclen */
static unsigned
count_bits(const void *dat, const size_t len) {
  const char *data = dat;
  unsigned count = 0;
  unsigned i,j;
  for (i = 0; i < len; i++) {
    char atom = data[i];
    if (!atom)
      continue;
    else {
      /* dutifully count the bits */
      for(j = 0; j < 8; j++)
	if (atom & (1 << j))
	  count++;
    }
  }
  return count;
}

/* Heuristic for marking a pip */
static int __attribute__((unused))
flag_pip(const pip_db_t *pipdb,
	 const state_t *s, const unsigned pip,
	 const unsigned ulen) {
  unsigned nbits;
  pip_ref_t *piprec = get_pip(pipdb, pip);
  /* Very simple heuristic: if there are less than two bits left in the
     state, then the pip is considered isolated */
  nbits = count_bits(s->unknown_data, ulen);
  if (nbits > 2)
    /* Heuristic: not correctly isolated */
    piprec->isolated = 0;
  else
    piprec->isolated = 1;

  if (piprec->isolated)
    return 0;
  return -1;
}

static int
flag_pip_aggressively(const pip_db_t *pipdb,
		      const state_t *s, const unsigned pip,
		      const unsigned ulen) {
  pip_ref_t *piprec = get_pip(pipdb,pip);
  /* Aggressive heuristic: it is sufficient for the pip to be isolated
     formally that we consider it isolated. The rationale behind this is
     that superfluous bits outside of this are not flagged in the xdl
     model, we won't discover them with the current data, and they won't
     be conflit with the xdl models we have anyways -- hopefully */
  piprec->isolated = 1;
  return 0;
}



/* new notion, a pip is 'grouped' if all configuration elements in a set
   are equivalent and indistinguishable -- this happens when all pips,
   after thorough isolation, have exactly the same left-hand-side (known
   state) */
static int
_is_grouped(const pip_db_t *pipdb,
	    const alldata_t *dat, const unsigned pip) {
  size_t len = dat->known_data_len;
  const state_t *work_state;
  char *kdata;
  unsigned i;

  work_state = get_pip_state(pipdb,pip);
  kdata = work_state->known_data;

  for (i=0; i < len; i++) {
    if (kdata[i]) {
      /* check that the i element has the same lefthand side */
      if (i != pip) {
	const state_t *state = get_pip_state(pipdb,i);
	fprintf(stderr, "comparing state %i and %i\n",i,pip);
	if (memcmp(kdata, state->known_data, len))
	  break;
      }
    }
  }
  if (i == len)
    return 0;
  return -1;
}

static unsigned
mark_group(const pip_db_t *pipdb, const alldata_t *dat, const unsigned pip) {
  size_t len = dat->known_data_len;
  unsigned marked = 0;
  const state_t *work_state;
  char *kdata;
  unsigned i;

  work_state = get_pip_state(pipdb, pip);
  kdata = work_state->known_data;

  for (i=0; i < len; i++) {
    if (kdata[i]) {
      pip_ref_t *piprec = get_pip(pipdb, pip);
      piprec->isolated = 1;
      marked++;
    }
  }
  return marked;
}

static unsigned
do_grouped_pips(const pip_db_t *pipdb, const alldata_t *dat) {
  /* mark as grouped pips which are in an equivalence class */
  const unsigned npips = pipdb->pip_num;
  unsigned added = 0;
  unsigned pip;

  /* allocated and zeroed */

  for(pip = 0; pip < npips; pip++) {
    /* If we're not doing the first thing, then */
    pip_ref_t *piprec = get_pip(pipdb, pip);

    fprintf(stderr,"doing pip #%08i, %s -> %s...",pip,
	    get_pip_start(pipdb, pip),get_pip_end(pipdb, pip));

    if (piprec->isolated) {
	fprintf(stderr,"pip already isolated\n");
	continue;
    }

    fprintf(stderr,"checking for grouped pip\n");
    if (!_is_grouped(pipdb,dat,pip)) {
      fprintf(stderr,"pip is grouped !\n");
      added += mark_group(pipdb,dat,pip);
    }
  }

  return added;
}

static void
remove_known_pips(const pip_db_t *pipdb,
		  const alldata_t *dat, const state_t *s, const unsigned kpip) {
  char *remains = s->known_data;
  unsigned len = dat->known_data_len;
  unsigned ulen = dat->unknown_data_len;
  unsigned i;
  /* run through the state's as-of-yet unseparated states,
   * and remove them if they are actually isolated
   */
  for(i = 0; i < len; i++) {
    pip_ref_t *pip = get_pip(pipdb, i);
    if (i != kpip && remains[i] && pip->isolated) {
      /* we can do something */
      fprintf(stderr, "remaining @%i can be wiped\n", i);
      /* clear our bit */
      remains[i]=0;
      and_neg_data(s->unknown_data, pip->state.unknown_data, ulen);
    }
  }
}

/* Do one iteration of the loop */
static unsigned
do_thorough_pips(const pip_db_t *pipdb,
		 const alldata_t *dat,
		 const int first_iteration) {
  unsigned npips = pipdb->pip_num;
  unsigned pip;
  core_status_t status;
  unsigned added = 0;
  size_t len = dat->known_data_len;
  size_t ulen = dat->unknown_data_len;

  /* allocated and zeroed */

  for(pip = 0; pip < npips; pip++) {
    const state_t *pip_state = get_pip_state(pipdb, pip);

    fprintf(stderr,"doing pip #%08i, %s -> %s...",pip,
	    get_pip_start(pipdb, pip),get_pip_end(pipdb, pip));

    status = isolate_bit_core(pip_state, dat, pip);

    if (first_iteration) {
      if (status != STATUS_NOTALONE)
	if (!flag_pip_aggressively(pipdb, pip_state, pip, ulen))
	  added++;
      fprintf(stderr,"done\n");
      continue;
    } else {
      /* If we're not doing the first thing, then */
      pip_ref_t *piprec = get_pip(pipdb, pip);
      if (piprec->isolated) {
	fprintf(stderr,"isolated at once\n");
	continue;
      }
    }
    fprintf(stderr,"doing big search\n");
    /* The we need to further hand-remove all already-known
       pips */
    remove_known_pips(pipdb,dat,pip_state,pip);
    /* again test for completion */
    if (!_is_isolated(stderr,pip,pip_state->known_data, len) &&
	!flag_pip_aggressively(pipdb,pip_state,pip,ulen)) {
      /* This worked ! Very good ! */
      fprintf(stderr,"It Worked ! New PIP %i isolated !\n", pip);
      added++;
    }
  }
  return added;
}

/* loop me */
static void
do_thorough_passes(const pip_db_t *pipdb,
		   alldata_t *dat) {
  size_t ulen = dat->unknown_data_len;
  int first_pass = 1;
  unsigned added, pass = 0;
  do {
    fprintf(stderr,"Doing pass %i...", pass);
    added = do_thorough_pips(pipdb, dat, first_pass);
    fprintf(stderr,"%i pips done\n",added);
    first_pass = 0;
    pass++;
  } while (added != 0);

  do_grouped_pips(pipdb, dat);

  /* redo a pass over this */
  do {
    fprintf(stderr,"Doing pass %i...", pass);
    added = do_thorough_pips(pipdb, dat, first_pass);
    fprintf(stderr,"%i pips done\n",added);
    first_pass = 0;
    pass++;
  } while (added != 0);

  /* Then dump the db of isolated pips */
  dump_pips_db(pipdb, ulen);
}

/* TODO: we can also do the _reverse_ : given an unknown bit, eliminate
   everything, and see what's left in the known data. This could prove
   interresting
*/

static char *start = NULL;
static char *end = NULL;
static gboolean allpips = FALSE;
static gboolean unite = FALSE;
static gboolean thorough = FALSE;

static const char **dat_output = NULL;
static const char **dat_input = NULL;


static GOptionEntry entries[] =
{
  {"input", 'i', 0, G_OPTION_ARG_FILENAME_ARRAY, &dat_input, "Input values of the function we seek", NULL},
  {"output", 'o', 0, G_OPTION_ARG_FILENAME_ARRAY, &dat_output, "Output values of the function we seek", NULL},
  {"thorough", 't', 0, G_OPTION_ARG_NONE, &thorough, "be thorough", NULL},
  {"union", 'u', 0, G_OPTION_ARG_NONE, &unite, "Unite !", NULL},
  { NULL }
};

static int do_real_work() {
  pip_db_t *pipdb;
  alldata_t *dat;

  pipdb = build_pip_db(dat_input);

  /* we fill the data and build the database along the way */
  dat = fill_all_data(pipdb, dat_input, dat_output);

  /* what is this now ? */
  alloc_pips_state(pipdb,
		   dat->known_data_len,
		   dat->unknown_data_len);

  /* */

  if (thorough) {
    do_thorough_passes(pipdb, dat);
    goto exit;
  }

  if (unite) {
    do_filtered_pips(pipdb, dat, start, end);
    goto exit;
  }

  /* Not exactly clever, more though for CL argument */
  if (allpips)
    do_all_pips(pipdb, dat);
/*   else { */
/*     /\* XXX -- allow filtering on start / end *\/ */
/*     isolate_bit(pip, dat); */
/*   } */

 exit:
  free_pips_state(pipdb);
  free_all_data(dat);

  print_summary();
  return 0;
}

/* Final function does the job */
int main(int argc, char *argv[], char *env[]) {
  GError *error = NULL;
  GOptionContext *context = NULL;

  context = g_option_context_new ("- find a set function given some input/output pairs");
  g_option_context_add_main_entries (context, entries, NULL);
  g_option_context_parse (context, &argc, &argv, &error);
  if (error != NULL) {
    g_warning("parse error: %s",error->message);
    g_error_free (error);
    return -1;
  }

  g_option_context_free(context);

  return do_real_work();
}
