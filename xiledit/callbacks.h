#include <gtk/gtk.h>

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
