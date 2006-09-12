/*
 * (C) Copyright 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
 * All rights reserved.
 */

#include <glib.h>
#include "keyfile.h"

gint
read_keyfile(GKeyFile **fill, const gchar *filename) {
  GKeyFile *db;
  GError *error = NULL;

//  g_log(G_LOG_DOMAIN, PIP_LOG_DATA, "Loading data from %s", filename);

  db = g_key_file_new();
  if (!db)
    goto out_err;

  g_key_file_load_from_file(db,filename,G_KEY_FILE_NONE,&error);

  if (error != NULL) {
    g_warning("could not read db %s: %s",filename,error->message);
    g_error_free (error);
    goto out_err_free;
  }

  *fill = db;
  return 0;

 out_err_free:
  g_key_file_free (db);
 out_err:
  return -1;
}

/** \brief Iterator over groups in keyfile
 *
 * @param pipdb the pip database
 * @param func the group iterator
 * @param closure the group iterator argument
 * @see group_hook_t
 */

void
iterate_over_groups(GKeyFile *db,
		    group_hook_t func,
		    gpointer closure) {
  gsize nends,i;
  gchar **cends;
  cends = g_key_file_get_groups(db, &nends);

  for(i=0; i < nends; i++) {
    const gchar *group = cends[i];
    func(db, group, closure);
  }

  g_strfreev(cends);
}

