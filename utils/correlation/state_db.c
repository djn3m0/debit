/*
 * (C) Copyright 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
 * All rights reserved.
 *
 */

#include <glib.h>
#include "bitisolation_db.h"

static int
get_file_data(gchar **data, unsigned *len, const gchar *file) {
  GError *error;
  if (g_file_get_contents (file, data, len, &error))
    return 0;

  g_warning("Could not load data from %s: %s", file,
	    error->message);
  g_error_free(error);
  return -1;
}

static void
set_pip_bit(const gchar *line, void *data) {
  pip_db_t *db = data;
  unsigned bit = get_pip_index(db, line);
  (void) bit;
  //set_bit();
}

static int
get_file_txtdata(const pip_db_t *db,
		 gchar **data, unsigned *len, const gchar *filename) {
  /* allocate the data array */
  /* fill the data array correctly */
  iterate_over_lines(filename, set_pip_bit, NULL);
  return 0;
}

/* Iterate over all sites to get all data */
alldata_t *
fill_all_data(const pip_db_t *db,
	      const gchar **knw, const gchar **uknw) {
  alldata_t *dat = g_new(alldata_t, 1);
  unsigned idx = 0;
  GArray *data_array;

  data_array = g_array_new(FALSE, FALSE, sizeof(state_t));

  while (knw[idx] != NULL && uknw[idx] != NULL) {
    const gchar *inp = knw[idx], *outp = uknw[idx];
    state_t s;
    /* XXX Check data length */
    get_file_txtdata(db, &s.known_data, &dat->known_data_len, inp);
    get_file_data(&s.unknown_data, &dat->unknown_data_len, outp);
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
