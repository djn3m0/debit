#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>

#include "callbacks.h"
#include "interface.h"
#include "support.h"

#include "xildraw.h"

#include "debitlog.h"

int display_bitstream(const gchar *filename);

void
on_open1_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  GtkWidget *parent;
  GtkWidget *dialog;

  parent = GTK_WIDGET(user_data);

  dialog = gtk_file_chooser_dialog_new ("Open bitstream",
					GTK_WINDOW(parent),
					GTK_FILE_CHOOSER_ACTION_OPEN,
					GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
					NULL);

  if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT) {
    gchar *filename;

    filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));

    if (filename)
      display_bitstream(filename);

    g_free (filename);
  }

  gtk_widget_destroy (dialog);

  return;
}

void
on_about1_activate                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  GtkWidget *about;

  about = create_about ();
  gtk_widget_show (about);

}


void
on_information1_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{

}

/* Zoom submenus */

void
on_zoom1_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{

}


void
on_zoom_out1_activate                  (gpointer         user_data,
					GtkMenuItem     *menuitem)
{
  /* get back to the xildraw container */
  GtkMenu * menu = GTK_MENU (user_data);
  EggXildrawFace *xildraw = EGG_XILDRAW_FACE(gtk_menu_get_attach_widget (menu));

  double zoom;

  zoom = gtk_adjustment_get_value(xildraw->zoomadjust);
  gtk_adjustment_set_value(xildraw->zoomadjust, zoom * 0.75);
}


void
on_zoom_in1_activate                   (gpointer         user_data,
					GtkMenuItem     *menuitem)
{
  GtkMenu * menu = GTK_MENU (user_data);
  EggXildrawFace *xildraw = EGG_XILDRAW_FACE(gtk_menu_get_attach_widget (menu));

  /* NB: should just emit a signal */
  double zoom;

  zoom = gtk_adjustment_get_value(xildraw->zoomadjust);
  gtk_adjustment_set_value(xildraw->zoomadjust, zoom * 1 / 0.75);
}

void
on_best_fit1_activate                  (gpointer         user_data,
					GtkMenuItem     *menuitem)
{
  GtkMenu * menu = GTK_MENU (user_data);
  EggXildrawFace *xildraw = EGG_XILDRAW_FACE(gtk_menu_get_attach_widget (menu));

  egg_xildraw_zoom_fit(xildraw);
}

void
on_fullscreen1_activate                (gpointer         user_data,
					GtkMenuItem     *menuitem)
{
  GtkMenu * menu = GTK_MENU (user_data);
  EggXildrawFace *xildraw = EGG_XILDRAW_FACE(gtk_menu_get_attach_widget (menu));
  egg_xildraw_fullscreen(xildraw);
  /* Have the window go fullscreen, then fit to screen */
}

void
on_unfullscreen1_activate              (gpointer         user_data,
					GtkMenuItem     *menuitem)
{
  GtkMenu * menu = GTK_MENU (user_data);
  EggXildrawFace *xildraw = EGG_XILDRAW_FACE(gtk_menu_get_attach_widget (menu));
  egg_xildraw_unfullscreen(xildraw);
}

/* Drawing control */

void
on_sites1_toggled                      (gpointer         user_data,
					GtkCheckMenuItem *checkmenuitem)
{

}


void
on_site_names1_toggle                  (gpointer         user_data,
                                        GtkCheckMenuItem *checkmenuitem)
{
  GtkMenu * menu = GTK_MENU (user_data);
  EggXildrawFace *xildraw = EGG_XILDRAW_FACE(gtk_menu_get_attach_widget (menu));
  egg_xildraw_site_names(xildraw, gtk_check_menu_item_get_active (checkmenuitem));
}

