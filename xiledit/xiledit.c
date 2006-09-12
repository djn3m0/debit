/*
 * (C) Copyright 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
 * All rights reserved.
 */

#include <gtk/gtk.h>
#include "xildraw.h"

int
main (int argc, char **argv)
{
	GtkWidget *window;
	GtkWidget *fpga;

	gtk_init (&argc, &argv);

	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

	fpga = egg_xildraw_face_new ();
	gtk_container_add (GTK_CONTAINER (window), fpga);

	g_signal_connect (window, "destroy",
			  G_CALLBACK (gtk_main_quit), NULL);

	gtk_widget_show_all (window);

	gtk_main ();

	return 0;
}


/* int */
/* main(int argc, char **argv) */
/* { */
/* 	GladeXML *xml; */
/* 	GtkWidget *widget; */

/* 	gtk_init(&argc, &argv); */
/* 	xml = glade_xml_new("xiledit.glade", NULL, NULL); */

/* 	/\* get a widget (useful if you want to change something) *\/ */
/* 	widget = glade_xml_get_widget(xml, "widgetname"); */

/* 	/\* connect signal handlers *\/ */
/* 	glade_xml_signal_autoconnect(xml); */

/* 	gtk_main(); */

/* 	return 0; */
/* } */
