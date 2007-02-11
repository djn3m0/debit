/*
 * Copyright (C) 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
 */

/** \file
    This is the command-line program to invoke the debit parser, glib
    version.

    @todo Switch to the new paradigm of using separate actions in
    debit_file, i.e., first ask for the bitstream to be parsed, then,
    according to flags, perform some actions. This will allow less
    clutter and hooks in the debit parser, will improve readability
    of the file, and will clarify the API. Pipdum already is an instance
    of this.

    \todo wrap mapping of file &al into a high-level function of the
    parser API

    \todo reorganize the data to seprate the bitstream data proper and
    the parsing structures. Some elements must be freed after parsing,
    but the data must not.
*/

#include <glib.h>

#include "bitstream_parser.h"
#include "debitlog.h"
#include "filedump.h"
#include "analysis.h"

static gboolean framedump = FALSE;
static gboolean sitedump = FALSE;
static gboolean unkdump = FALSE;
static gboolean pipdump = FALSE;
static gboolean lutdump = FALSE;
static gboolean bramdump = FALSE;
static gboolean netdump = FALSE;

static gchar *ifile = NULL;
static gchar *odir = "";
static gchar *datadir = DATADIR;
static gchar *suffix = ".bin";

#if DEBIT_DEBUG > 0
unsigned int debit_debug = 0;
#else
static unsigned debit_local_debug;
#endif

static int
debit_file(gchar *input_file, gchar *output_dir) {
  gint err = 0;
  bitstream_parsed_t *bit;

  bit = parse_bitstream(input_file);

  if (bit == NULL) {
    err = -1;
    goto out;
  }

  /* Have some action */
  if (framedump)
    design_write_frames(bit, output_dir);

  if (unkdump)
    design_dump_frames(bit, output_dir);

  if (sitedump || pipdump || lutdump || bramdump || netdump) {
    bitstream_analyzed_t *analysis = analyze_bitstream(bit, datadir);
    if (analysis == NULL)
      goto out_free;

/*     print_chip(analysis->chip); */

    if (sitedump)
      dump_sites(analysis, output_dir, suffix);
    if (pipdump)
      dump_pips(analysis);
    if (lutdump)
      dump_luts(analysis);
    if (bramdump)
      dump_bram(analysis);
    if (netdump)
      dump_nets(analysis);

    free_analysis(analysis);
  }

 out_free:
  free_bitstream(bit);
 out:
  return err;
}

static GOptionEntry entries[] =
{
  {"input", 'i', 0, G_OPTION_ARG_FILENAME, &ifile, "Read bitstream <ifile>", "<ifile>"},
#if DEBIT_DEBUG > 0
  {"debug", 'g', 0, G_OPTION_ARG_INT, &debit_debug, "Debug verbosity", NULL},
#else
  {"debug", 'g', 0, G_OPTION_ARG_INT, &debit_local_debug, "Debug verbosity", NULL},
#endif
  {"outdir", 'o', 0, G_OPTION_ARG_FILENAME, &odir, "Write data files in directory <odir>", "<odir>"},
  {"datadir", 'd', 0, G_OPTION_ARG_FILENAME, &datadir, "Read data files from directory <datadir>", "<datadir>"},
  /* v2 specific */
  {"framedump", 'f', 0, G_OPTION_ARG_NONE, &framedump, "Dump raw data frames", NULL},
  {"sitedump", 's', 0, G_OPTION_ARG_NONE, &sitedump, "Dump raw site data files", NULL},
  {"suffix", 'x', 0, G_OPTION_ARG_STRING, &suffix, "Suffix appended to generated files",NULL},
  /* v4, v5 specific */
  {"unkdump", 'u', 0, G_OPTION_ARG_NONE, &unkdump, "Dump raw data frames uninterpreted", NULL},
  {"pipdump", 'p', 0, G_OPTION_ARG_NONE, &pipdump, "Dump pips in the bitstream", NULL},
  {"lutdump", 'l', 0, G_OPTION_ARG_NONE, &lutdump, "Dump lut data from the bitstream", NULL},
  {"bramdump", 'b', 0, G_OPTION_ARG_NONE, &bramdump, "Dump bram data from the bitstream", NULL},
  {"netdump", 'n', 0, G_OPTION_ARG_NONE, &netdump, "Dump nets rebuilt from the bitstream (experimental)", NULL},
  { NULL, '\0', 0, 0, 0, NULL, NULL }
};

int
main(int argc, char *argv[])
{
  int err;
  GError *error = NULL;
  GOptionContext *context = NULL;

  context = g_option_context_new ("- dump xilinx bitstream data");
  g_option_context_add_main_entries (context, entries, NULL);
  g_option_context_parse (context, &argc, &argv, &error);
  if (error != NULL) {
    g_warning("parse error: %s",error->message);
    g_error_free (error);
    return -1;
  }

  g_option_context_free(context);

  if (!ifile) {
    g_warning("You must specify a bitfile, %s --help for help", argv[0]);
    return -1;
  }

  err = debit_file(ifile, odir);
  return err;
}
