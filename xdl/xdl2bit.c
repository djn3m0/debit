/*
 * (C) Copyright 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
 * All rights reserved.
 */

#include <stdio.h>
#include <glib.h>

#include <stdint.h>
#include "xdl_parser.h"
#include "parser.h"
#include "bitstream_write.h"

/* Parsing, lexing */
extern int yyparse (void *);
extern int yylex_destroy (void);
extern FILE* yyin;

/* Command-line */
static gchar *ifile = NULL;
static gchar *ofile = NULL;
static gchar *odir = "";
static gchar *datadir = DATADIR;

#if DEBIT_DEBUG > 0
unsigned int debit_debug = 0;
#endif

static GOptionEntry entries[] =
{
  {"input", 'i', 0, G_OPTION_ARG_FILENAME, &ifile, "Read XDL file <ifile>", "<ifile>"},
  {"output", 'b', 0, G_OPTION_ARG_FILENAME, &ofile, "Write bitstream to file <ofile>", "<ofile>"},
  {"outdir", 'o', 0, G_OPTION_ARG_FILENAME, &odir, "Write data files in directory <odir>", "<odir>"},
#if DEBIT_DEBUG > 0
  {"debug", 'g', 0, G_OPTION_ARG_INT, &debit_debug, "Debug verbosity", NULL},
#endif
  {"datadir", 'd', 0, G_OPTION_ARG_FILENAME, &datadir, "Read data files from directory <datadir>", "<datadir>"},
  { NULL, '\0', 0, 0, NULL, NULL, NULL }
};

static int
xdl2bit_init(int *argcp, char ***argvp) {
  GError *error = NULL;
  GOptionContext *context = NULL;
  int err = 0;

  context = g_option_context_new ("- turn a XDL file into a nice bitstream");
  g_option_context_add_main_entries (context, entries, NULL);
  g_option_context_parse (context, argcp, argvp, &error);
  if (error != NULL) {
    g_warning("cmdline parse error: %s", error->message);
    g_error_free (error);
    return 1;
  }

  if (ifile) {
    FILE *file = fopen(ifile, "r");
    if (!file)
      err = -1;
    else
      yyin = file;
  } else {
    yyin = stdin;
  }

  g_option_context_free(context);
  return err;
}

extern int yydebug;

int main(int argc, char **argv) {
	parser_t parser = {
		.pip_counter = 0,
		.pipdb = NULL,
		/* parser state */
		.current_site = NULL,
		.slice_idx = 0,
	};
	int err;

/* 	yydebug = 1; */

	err = xdl2bit_init(&argc, &argv);
	if (err)
	  return err;
	parser.datadir = datadir,

	err = yyparse(&parser);
	if (err) {
	  g_warning("XDL parse error");
	  /* XXX Free on error path */
	  return err;
	}

	debit_log(L_PARSER, "Processed %u pips\n", parser.pip_counter);

	/* Generate bitstream, if asked for */
	if (ofile)
	  bitstream_write(&parser.bit, odir, ofile);

	/* XXX Free */
	free_parser(&parser);
	(void) yylex_destroy();

	return 0;
}
