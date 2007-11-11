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

#include <gtk/gtk.h>

#include "debitlog.h"
#include "xildraw.h"
#include "analysis.h"

/* Glade-generated files */
#include "interface.h"
#include "support.h"

static gchar *ifile = NULL;
static gchar *datadir = DATADIR;

static void glade_do_init(void) {
  GtkWidget *menu;
/*   GtkWidget *birdseye; */

 /*
  *    * The following code was added by Glade to create one of each component
  *    * (except popup menus), just so that you see something after building
  *    * the project. Delete any components that you don't want shown initially.
  *
  */
  gchar *pixmapsdir = g_build_filename(datadir, "pixmaps", NULL);
  add_pixmap_directory (pixmapsdir);
  g_free(pixmapsdir);

  menu = create_menu ();
  gtk_widget_show (menu);
/*   birdseye = create_birdseye (); */
/*   gtk_widget_show (birdseye); */
  return;
}

static void
destroy_window(GtkWidget *win, gpointer data) {
  bitstream_analyzed_t *nlz = data;
  (void) win;
  free_analysis(nlz);
}

static int
display_window(bitstream_analyzed_t *nlz,
	       const gchar *filename) {
  GtkWidget *window;
  GtkWidget *fpga;
  gchar *title = NULL;

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

  fpga = egg_xildraw_face_new (nlz);
  gtk_container_add (GTK_CONTAINER (window), fpga);

  g_signal_connect (window, "destroy", G_CALLBACK(destroy_window), nlz);

  title = g_strdup_printf("[%s]", filename);
  gtk_window_set_title (GTK_WINDOW(window), title);
  g_free(title);

  egg_xildraw_adapt_window(EGG_XILDRAW_FACE(fpga), GTK_WINDOW(window));

  gtk_widget_show_all (window);
  return 0;
}

/*
 * Commandline arguments
 */


#if DEBIT_DEBUG > 0
unsigned int debit_debug = 0;
#endif

static GOptionEntry entries[] =
{
  {"input", 'i', 0, G_OPTION_ARG_FILENAME, &ifile, "Read bitstream <ifile>", "<ifile>"},
#if DEBIT_DEBUG > 0
  {"debug", 'g', 0, G_OPTION_ARG_INT, &debit_debug, "Debug verbosity", NULL},
#endif
  {"datadir", 'd', 0, G_OPTION_ARG_FILENAME, &datadir, "Read data files from directory <datadir>", "<datadir>"},
  { NULL }
};

extern int
display_bitstream(const gchar *filename);

int
display_bitstream(const gchar *filename) {
  bitstream_parsed_t *bit;
  bitstream_analyzed_t *nlz;

  bit = parse_bitstream(filename);
  if (!bit) {
    g_warning("Could not parse the bitfile");
    return -1;
  }

  nlz = analyze_bitstream(bit, datadir);
  if (!nlz) {
    g_warning("Could not analyze the bitfile");
    return -1;
  }

  return display_window (nlz, filename);
}

static void
debit_init(int *argcp, char ***argvp) {
  GError *error = NULL;
  GOptionContext *context = NULL;

  context = g_option_context_new ("- browse xilinx bitstream");
  g_option_context_add_main_entries (context, entries, NULL);
  g_option_context_parse (context, argcp, argvp, &error);
  if (error != NULL) {
    g_warning("parse error: %s", error->message);
    g_error_free (error);
  }
  g_option_context_free(context);
}

int
main (int argc, char **argv) {
  gtk_init (&argc, &argv);
  debit_init (&argc, &argv);

  /* glade-generated widgets */
  glade_do_init ();

  if (ifile)
    display_bitstream(ifile);

  gtk_main ();

  return 0;
}
