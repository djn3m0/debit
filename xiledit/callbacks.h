#include <gtk/gtk.h>

/* These should be in private scope */

void
on_open1_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_quit1_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_about1_activate                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_information1_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_zoom1_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_zoom_out1_activate                  (gpointer         user_data,
					GtkMenuItem     *menuitem);

void
on_zoom_in1_activate                   (gpointer         user_data,
					GtkMenuItem     *menuitem);

void
on_best_fit1_activate                  (gpointer         user_data,
					GtkMenuItem     *menuitem);

void
on_fullscreen1_activate                (gpointer         user_data,
					GtkMenuItem     *menuitem);

void
on_unfullscreen1_activate              (gpointer         user_data,
					GtkMenuItem     *menuitem);
