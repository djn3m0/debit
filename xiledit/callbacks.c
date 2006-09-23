#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>

#include "callbacks.h"
#include "interface.h"
#include "support.h"

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

