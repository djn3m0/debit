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

#include "debitlog.h"
#include "bitisolation_db.h"
#include "algos.h"

/* TODO: we can also do the _reverse_ : given an unknown bit, eliminate
   everything, and see what's left in the known data. This could prove
   interresting
*/

static gboolean allelems = FALSE;
static gboolean thorough = FALSE;
static gboolean iterate = FALSE;
static gboolean internal = FALSE;
static guint width = 0;

static const char *ref = NULL;
static const char **data = NULL;

#if DEBIT_DEBUG > 0
unsigned int debit_debug = L_ANY;
#endif

static GOptionEntry entries[] =
{
  {"data", 'd', 0, G_OPTION_ARG_FILENAME_ARRAY, &data,
   "read *.bin and *.dat input/output pairs of the binary function from <datadir>",
   "datadir"},
#if DEBIT_DEBUG > 0
  {"debug", 'g', 0, G_OPTION_ARG_INT, &debit_debug, "Debug verbosity", NULL},
#endif
  {"ref", 'r', 0, G_OPTION_ARG_FILENAME, &ref, "take <zero>.bin as reference bit data (image of zero)", "<zero>"},
  {"width", 'w', 0, G_OPTION_ARG_INT, &width, "set the width of the data for 2D representation of the data", NULL},
  {"allelems", 'a', 0, G_OPTION_ARG_NONE, &allelems, "isolate by intersection", NULL},
  {"thorough", 't', 0, G_OPTION_ARG_NONE, &thorough, "isolate by intersection and negation-intersection", NULL},
  {"iterate", 'i', 0, G_OPTION_ARG_NONE, &iterate, "iterate the algorithm until a fixed point is reached", NULL},
  {"internal", 'n', 0, G_OPTION_ARG_NONE, &internal, "allow internal composition of the states (long)", NULL},
  { NULL }
};

static int do_real_work() {
  pip_db_t *pipdb;
  alldata_t *dat;

  pipdb = build_pip_db(data);
  dat = fill_all_data(pipdb, ref, data);
  dat->width = width;

  /*
    Allocate the pip db state.
    Maybe this could be done at build_pip_db time, or use another
    dedicated structure.
  */
  alloc_pips_state(pipdb, dat);

  /* Work in full algebra -- this takes a long time */
  if (internal) {
    do_all_pips_internal(pipdb, dat, iterate);
    goto dump;
  }

  /* Intersect and conter-interset pips */
  if (thorough) {
    do_all_pips_thorough(pipdb, dat, iterate);
    goto dump;
  }

  /* Dumbest form of isolation, interset only */
  if (allelems) {
    do_all_pips(pipdb, dat);
    goto dump;
  }

  goto exit;

 dump:
  dump_pips_db(pipdb);
 exit:
  free_pips_state(pipdb);
  free_all_data(dat);
  free_pip_db(pipdb);
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
