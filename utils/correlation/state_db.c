/*
 * (C) Copyright 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
 * All rights reserved.
 *
 */

#include <glib.h>
#include "altera/bitarray.h"
#include "bitisolation_db.h"

static int
get_file_bindata(bitarray_t **data, unsigned *len, const gchar *file) {
  gchar *filename = g_strconcat(file,".dat",NULL);
  gboolean done;
  GError *error;
  char *bindata;

  done = g_file_get_contents (filename, &bindata, len, &error);
  g_free(filename);

  if (done) {
    *data = bitarray_create_data(bindata, *len);
    return 0;
  }

  g_warning("Could not load data from %s: %s", filename,
	    error->message);
  g_error_free(error);
  return -1;
}

typedef struct _set_pip_arg {
  const pip_db_t *pipdb;
  bitarray_t *data;
} set_pip_arg_t;

static void
set_pip_bit(const gchar *line, void *data) {
  set_pip_arg_t *arg = data;
  const pip_db_t *db = arg->pipdb;
  bitarray_t *dat = arg->data;
  unsigned bit = get_pip_index(db, line);
  bitarray_set(dat,bit);
}

static int
get_file_txtdata(const pip_db_t *db, bitarray_t **data, const gchar *file) {
  gchar *filename = g_strconcat(file,".dat",NULL);
  set_pip_arg_t arg = { .pipdb = db };
  bitarray_t *bindata;

  /* allocate the data array */
  bindata = bitarray_create(db->pip_num);
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

  dat->known_data_len = BYTES_OF_BITS(db->pip_num);
  dat->nstates = data_array->len;
  dat->states = (state_t *)g_array_free(data_array, FALSE);

  return dat;
}

void
free_all_data(alldata_t *dat) {
  g_free(dat->states);
  g_free(dat);
}
