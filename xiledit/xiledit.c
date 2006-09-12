/*
 * (C) Copyright 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
 * All rights reserved.
 */

#include <gtk/gtk.h>
#include <glade/glade-xml.h>
#include <glade/glade-init.h>
#include "xildraw.h"

static void glade_do_init() {
  GladeXML *xml;

  xml = glade_xml_new("xiledit.glade", NULL, NULL);
  glade_xml_signal_autoconnect(xml);

  /* Glade does not (yet) handle our custom widget */

  return;
}

static void
display_window() {
  GtkWidget *window;
  GtkWidget *fpga;

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

  fpga = egg_xildraw_face_new ();
  gtk_container_add (GTK_CONTAINER (window), fpga);

  g_signal_connect (window, "destroy",
		    G_CALLBACK (gtk_main_quit), NULL);

  gtk_widget_show_all (window);
}

int
main (int argc, char **argv)
{
  gtk_init (&argc, &argv);

  /* glade-generated widgets */
  glade_do_init ();
  /* custom windows */
  display_window ();
  gtk_main ();

  return 0;
}
