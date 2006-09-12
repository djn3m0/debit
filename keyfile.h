/*
 * (C) Copyright 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
 * All rights reserved.
 */

#include <glib.h>

gint read_keyfile(GKeyFile **fill, const gchar *filename);

typedef void (*group_hook_t)(GKeyFile *, const gchar *, gpointer);
void iterate_over_groups(GKeyFile *db, group_hook_t func, gpointer closure);
