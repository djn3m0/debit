/*
 * (C) Copyright 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
 * All rights reserved.
 *
 */

#include <glib.h>
#include "bitisolation_db.h"

static int
get_file_bindata(gchar **data, unsigned *len, const gchar *file) {
  gchar *filename = g_strconcat(file,".dat",NULL);
  gboolean done;
  GError *error;

  done = g_file_get_contents (filename, data, len, &error);
  g_free(filename);

  if (done)
    return 0;

  g_warning("Could not load data from %s: %s", filename,
	    error->message);
  g_error_free(error);
  return -1;
}

typedef struct _set_pip_arg {
  const pip_db_t *pipdb;
  char *data;
} set_pip_arg_t;

static void
set_pip_bit(const gchar *line, void *data) {
  set_pip_arg_t *arg = data;
  const pip_db_t *db = arg->pipdb;
  unsigned bit = get_pip_index(db, line);
  (void) bit;
  //set_bit();
}

static int
get_file_txtdata(const pip_db_t *db, gchar **data, const gchar *file) {
  gchar *filename = g_strconcat(file,".dat",NULL);
  set_pip_arg_t arg = { .pipdb = db };
  gchar *bindata;

  /* allocate the data array */
  bindata = g_malloc0(db->pip_num);
  arg.data = bindata;

  /* fill the data array correctly */
  iterate_over_lines(filename, set_pip_bit, &arg);
  g_free(filename);

  *data = bindata;

  return 0;
}

/* Iterate over all sites to get all data */
alldata_t *
fill_all_data(const pip_db_t *db, const gchar **knw) {
  alldata_t *dat = g_new(alldata_t, 1);
  unsigned idx = 0;
  GArray *data_array;

  data_array = g_array_new(FALSE, FALSE, sizeof(state_t));

  while (knw[idx] != NULL) {
    const gchar *inp = knw[idx];
    state_t s;
    /* XXX Check data length */
    get_file_txtdata(db, &s.known_data, inp);
    get_file_bindata(&s.unknown_data, &dat->unknown_data_len, inp);
    g_array_append_val(data_array, s);
    idx++;
  }

  dat->nstates = data_array->len;
  dat->states = (state_t *)g_array_free(data_array, FALSE);

  return dat;
}

void
free_all_data(alldata_t *dat) {
  g_free(dat->states);
  g_free(dat);
}
