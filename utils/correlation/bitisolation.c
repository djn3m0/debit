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
#include "algos.h"

/* TODO: we can also do the _reverse_ : given an unknown bit, eliminate
   everything, and see what's left in the known data. This could prove
   interresting
*/

static char *start = NULL;
static char *end = NULL;
static gboolean allelems = FALSE;
static gboolean unite = FALSE;
static gboolean thorough = FALSE;

static const char **data = NULL;

static GOptionEntry entries[] =
{
  {"data", 'd', 0, G_OPTION_ARG_FILENAME_ARRAY, &data,
   "<names>.bin and <names>.dat are input/output pairs of the binary function",
   "names"},
  {"allelems", 'a', 0, G_OPTION_ARG_NONE, &allelems, "try to isolate all elements", NULL},
  {"thorough", 't', 0, G_OPTION_ARG_NONE, &thorough, "be thorough", NULL},
  {"union", 'u', 0, G_OPTION_ARG_NONE, &unite, "Unite !", NULL},
  { NULL }
};

static int do_real_work() {
  pip_db_t *pipdb;
  alldata_t *dat;

  pipdb = build_pip_db(data);

  /* we fill the data and build the database along the way */
  dat = fill_all_data(pipdb, data);

  /* what is this now ? */
  alloc_pips_state(pipdb,
		   dat->known_data_len,
		   dat->unknown_data_len);

  /* real action */

/*   if (thorough) { */
/*     do_thorough_passes(pipdb, dat); */
/*     goto exit; */
/*   } */

  if (unite) {
    do_filtered_pips(pipdb, dat, start, end);
    goto exit;
  }

  /* Not exactly clever, more though for CL argument */
  if (allelems)
    do_all_pips(pipdb, dat);
/*   else { */
/*     /\* XXX -- allow filtering on start / end *\/ */
/*     isolate_bit(pip, dat); */
/*   } */

 exit:
  free_pips_state(pipdb);
  free_all_data(dat);

/*   print_summary(); */
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
