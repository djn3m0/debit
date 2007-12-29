/*
 * Copyright (C) 2006, 2007 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
 *
 * This file is part of debit.
 *
 * Debit is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Debit is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with debit.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <cairo.h>
#ifdef PDF_CAIRO
#include <cairo-pdf.h>
#endif /* PDF_CAIRO */
#ifdef PS_CAIRO
#include <cairo-ps.h>
#endif /* PS_CAIRO */
#ifdef SVG_CAIRO
#include <cairo-svg.h>
#endif /* SVG_CAIRO */
#include <string.h>

#include "debitlog.h"
#include "sites.h"
#include "bitdraw.h"

static gchar *ifile = NULL;
static gchar *ofile = NULL;
static gchar *datadir = DATADIR;
static gchar *otype = NULL;
static int width = 0;
static int height = 0;
static int dpi = 600;

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
  {"type", 't', 0, G_OPTION_ARG_STRING, &otype, "Type of output [pdf, ps, png]", NULL},
  {"width", 'w', 0, G_OPTION_ARG_INT, &width, "[png] width of image", NULL},
  {"height", 'l', 0, G_OPTION_ARG_INT, &height, "[png] height of image", NULL},
  {"dpi", 'r', 0, G_OPTION_ARG_INT, &dpi, "[pdf,ps] dpi resolution", NULL},
  {"datadir", 'd', 0, G_OPTION_ARG_FILENAME, &datadir, "Read data files from directory <datadir>", "<datadir>"},
  { NULL, '\0', 0, 0, NULL, NULL, NULL }
};

static int
bit2pdf_init(int *argcp, char ***argvp) {
  GError *error = NULL;
  GOptionContext *context = NULL;
  int err = 0;

  context = g_option_context_new ("- turn a xilinx bitstream into a nice drawing");
  g_option_context_add_main_entries (context, entries, NULL);
  g_option_context_parse (context, argcp, argvp, &error);
  if (error != NULL) {
    g_warning("commandline parse error: %s", error->message);
    g_error_free (error);
    err = -1;
  }

  g_option_context_free(context);
  return err;
}

typedef enum _out_type {
  PNG, PS, PDF, SVG, OTYPE_END,
} out_type_t;

static const char *tnames[OTYPE_END] = {
  [PNG] = "png",
  [PS] = "ps",
  [PDF] = "pdf",
  [SVG] = "svg",
};

static out_type_t get_optype(const char *lotype) {
  int i;
  if (!lotype)
    return PNG;

  for (i = 0; i < OTYPE_END; i++) {
    if (!strcmp(lotype,tnames[i]))
      return i;
  }
  g_warning("unknown requested type %s, selecting PNG", lotype);
  return PNG;
}

static int
draw_bitstream(const bitstream_analyzed_t *nlz, const gchar *lofile,
	       const out_type_t out) {
  chip_descr_t *chip = nlz->chip;
  cairo_surface_t *sr;
  cairo_t *cr;
  int err = 0;

  /* extract size from chip size */
  switch (out) {
#ifdef PDF_CAIRO
  case PDF:
    sr = cairo_pdf_surface_create (lofile,
				   chip->width * SITE_WIDTH,
				   chip->height * SITE_HEIGHT);
    break;
#endif /* PDF_CAIRO */
#ifdef PS_CAIRO
  case PS:
    sr = cairo_ps_surface_create (lofile,
				  chip->width * SITE_WIDTH,
				  chip->height * SITE_HEIGHT);
    break;
#endif /* PS_CAIRO */
#ifdef SVG_CAIRO
  case SVG:
    sr = cairo_svg_surface_create (lofile,
				   chip->width * SITE_WIDTH,
				   chip->height * SITE_HEIGHT);
    break;
#endif /* SVG_CAIRO */
  case PNG:
    sr = cairo_image_surface_create (CAIRO_FORMAT_RGB24,
				     chip->width * SITE_WIDTH,
				     chip->height * SITE_HEIGHT);
    break;
  default:
    return -1;
  }

  cairo_surface_set_fallback_resolution (sr, dpi, dpi);
  cr = cairo_create(sr);

  draw_cairo_chip(cr, chip);
  draw_cairo_wires(cr, nlz);

  switch (out) {
#if defined PDF_CAIRO || defined PS_CAIRO || defined SVG_CAIRO
  case PDF:
  case PS:
  case SVG:
    cairo_surface_flush(sr);
    cairo_show_page(cr);
    break;
#endif /* PDF_CAIRO || PS_CAIRO */
  case PNG:
    (void) cairo_surface_write_to_png (sr, lofile);
    break;
  default:
    err = -1;
  }

  cairo_destroy(cr);
  cairo_surface_destroy(sr);
  return err;
}

int main (int argc, char **argv) {
  bitstream_parsed_t *bit;
  bitstream_analyzed_t *nlz;
  out_type_t optype = PNG;
  int err;

  err = bit2pdf_init (&argc, &argv);
  if (err) {
    return err;
  }

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

  optype = get_optype(otype);
  g_warning("Creating %s document", tnames[optype]);

  err = draw_bitstream(nlz, ofile, optype);

  free_analysis(nlz);
  return err;
}
