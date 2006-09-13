/*
 * (C) Copyright 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
 * All rights reserved.
 */

#include <gtk/gtk.h>
#include <glade/glade-xml.h>
#include <glade/glade-init.h>

#include "debitlog.h"
#include "xildraw.h"
#include "analysis.h"

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

static void glade_do_init() {
  GladeXML *xml;

  xml = glade_xml_new("xiledit.glade", NULL, NULL);
  glade_xml_signal_autoconnect(xml);

  /* Glade does not (yet) handle our custom widget */

  return;
}

static void
display_window(bitstream_analyzed_t *nlz) {
  GtkWidget *window;
  GtkWidget *fpga;

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

  fpga = egg_xildraw_face_new (nlz);
  gtk_container_add (GTK_CONTAINER (window), fpga);

  g_signal_connect (window, "destroy",
		    G_CALLBACK (gtk_main_quit), NULL);

  gtk_widget_show_all (window);
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
   bitstream_parsed_t *bit;
   bitstream_analyzed_t *nlz;

   gtk_init (&argc, &argv);
   debit_init (&argc, &argv);

   if (!ifile) {
     g_warning("You must specify a bitfile, %s --help for help", argv[0]);
     return -1;
   }

   /* glade-generated widgets */
   glade_do_init ();

   /* custom widget, pending for glade integration */
   bit = parse_bitstream(ifile);
   nlz = analyze_bitstream(bit, datadir);
   display_window (nlz);

   gtk_main ();

   return 0;
}
