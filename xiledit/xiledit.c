/*
 * (C) Copyright 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
 * All rights reserved.
 */

#include <gtk/gtk.h>

#include "debitlog.h"
#include "xildraw.h"
#include "analysis.h"

/* Glade-generated files */
#include "interface.h"
#include "support.h"

static void glade_do_init() {
  GtkWidget *menu;
/*   GtkWidget *birdseye; */

 /*
  *    * The following code was added by Glade to create one of each component
  *    * (except popup menus), just so that you see something after building
  *    * the project. Delete any components that you don't want shown initially.
  *
  */
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

  gtk_widget_show_all (window);
  return 0;
}

/*
 * Commandline arguments
 */

static gchar *ifile = NULL;
static gchar *datadir = DATADIR;

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
