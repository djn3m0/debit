/*
 * (C) Copyright 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
 * All rights reserved.
 */

#include <stdio.h>
#include <glib.h>

#include "xdl_parser.h"
#include "parser.h"

/* Parsing */
extern int yyparse (void *);

/* Command-line */

static gchar *ifile = NULL;
static gchar *ofile = NULL;
static gchar *datadir = DATADIR;

#if DEBIT_DEBUG > 0
unsigned int debit_debug = 0;
#endif

static GOptionEntry entries[] =
{
  {"input", 'i', 0, G_OPTION_ARG_FILENAME, &ifile, "Read bitstream <ifile>", "<ifile>"},
  {"output", 'o', 0, G_OPTION_ARG_FILENAME, &ofile, "Write pdf to file <ofile>", "<ofile>"},
#if DEBIT_DEBUG > 0
  {"debug", 'g', 0, G_OPTION_ARG_INT, &debit_debug, "Debug verbosity", NULL},
#endif
  {"datadir", 'd', 0, G_OPTION_ARG_FILENAME, &datadir, "Read data files from directory <datadir>", "<datadir>"},
  { NULL, '\0', 0, 0, 0, NULL, NULL }
};

static void
xdl2bit_init(int *argcp, char ***argvp) {
  GError *error = NULL;
  GOptionContext *context = NULL;

  context = g_option_context_new ("- turn a XDL file into a nice bitstream");
  g_option_context_add_main_entries (context, entries, NULL);
  g_option_context_parse (context, argcp, argvp, &error);
  if (error != NULL) {
    g_warning("cmdline parse error: %s", error->message);
    g_error_free (error);
  }

  g_option_context_free(context);
}

int main(int argc, char **argv) {
	parser_t parser = {
		.pip_counter = 0,
		.design = NULL,
		.pipdb = NULL,
	};

	xdl2bit_init(&argc, &argv);
	parser.datadir = datadir,

	yyparse(&parser);

	printf("Processed %u pips\n", parser.pip_counter);

	return 0;
}
