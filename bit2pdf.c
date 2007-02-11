/*
 * (C) Copyright 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
 * All rights reserved.
 */

#include <cairo.h>
#include <cairo-pdf.h>
#include "debitlog.h"
#include "sites.h"
#include "bitdraw.h"

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
debit_init(int *argcp, char ***argvp) {
  GError *error = NULL;
  GOptionContext *context = NULL;

  context = g_option_context_new ("- turn a xilinx bitstream into a nice pdf");
  g_option_context_add_main_entries (context, entries, NULL);
  g_option_context_parse (context, argcp, argvp, &error);
  if (error != NULL) {
    g_warning("parse error: %s", error->message);
    g_error_free (error);
  }

  g_option_context_free(context);
}

static void
draw_pdf_bitstream(const bitstream_analyzed_t *nlz, const gchar *ofile) {
  chip_descr_t *chip = nlz->chip;
  cairo_surface_t *sr;
  cairo_t *cr;

  /* extract size from chip size */
  sr = cairo_pdf_surface_create (ofile,
				 chip->width * SITE_WIDTH,
				 chip->height * SITE_HEIGHT);
  cairo_surface_set_fallback_resolution (sr, 600, 600);

  cr = cairo_create(sr);

  draw_cairo_chip(cr, chip);
  draw_cairo_wires(cr, nlz);

  cairo_surface_flush(sr);
  cairo_show_page(cr);
  cairo_destroy(cr);

  cairo_surface_destroy(sr);
}

/* main, test function */

/* small testing utility */
int main (int argc, char **argv) {
  bitstream_parsed_t *bit;
  bitstream_analyzed_t *nlz;

  debit_init (&argc, &argv);

  if (!ifile) {
    g_warning("You must specify an input bitfile, %s --help for help", argv[0]);
    return -1;
  }

  if (!ofile) {
    g_warning("You must specify an output pdf, %s --help for help", argv[0]);
    return -1;
  }

  bit = parse_bitstream(ifile);
  if (!bit) {
    g_warning("Could not parse the bitfile");
    return -1;
  }

  nlz = analyze_bitstream(bit, datadir);
  if (!nlz) {
    g_warning("Could not analyze the bitfile");
    return -1;
  }

  /* cairo thingy */
  draw_pdf_bitstream(nlz, ofile);

  free_analysis(nlz);
  return 0;
}
