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
