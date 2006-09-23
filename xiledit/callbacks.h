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

void
on_sites1_activate                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_site_names1_activate                (gpointer         user_data,
					GtkMenuItem     *menuitem);

void
on_wires1_activate                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_wire_names1_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_sites1_deselect                     (GtkItem         *item,
                                        gpointer         user_data);

void
on_sites1_select                       (GtkItem         *item,
                                        gpointer         user_data);

void
on_sites1_toggled                      (gpointer         user_data,
					GtkCheckMenuItem *checkmenuitem);

void
on_site_names1_toggle                  (gpointer         user_data,
					GtkCheckMenuItem *checkmenuitem);
