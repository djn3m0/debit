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
  gchar *filename = g_strconcat(file,".bin",NULL);
  gboolean done;
  GError *error = NULL;
  char *bindata;

  done = g_file_get_contents (filename, &bindata, len, &error);
  g_free(filename);

  if (done) {
    *data = bitarray_create_data(bindata, *len * 8);
    return 0;
  }

  g_warning("Could not load bin data for %s: %s",
	    file, error->message);
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
  g_print("bitindex of %s is %i\n", line, bit);
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

static int
fill_state(state_t *s, const gchar *inp,
	   const pip_db_t *db) {
  bitarray_t *known, *unknown;
  unsigned udl;
  int err;

  err = get_file_txtdata(db, &known, inp);
  if (err) {
    g_warning("Error processing txt data for %s, skipping...", inp);
    goto out;
  }

  err = get_file_bindata(&unknown, &udl, inp);
  if (err) {
    g_warning("Error processing bin data for %s, skipping...", inp);
    goto out_free;
  }

  s->known_data = known;
  s->unknown_data = unknown;
  return 0;

 out_free:
  bitarray_free(known, FALSE);
 out:
  return err;

}

static void
process_data(GArray *data_array, const gchar *inp,
	     const bitarray_t *ref, const pip_db_t *db) {
  state_t s;
  int err;

  err = fill_state(&s, inp, db);

  if (!err) {
    unsigned bitcount;
    bitcount = bitarray_ones_count(s.unknown_data);
    g_print("Successfully loaded data from %s, %i bits set, ", inp, bitcount);

    if (ref) {
      bitarray_diffsym(s.unknown_data, ref);
      bitcount = bitarray_ones_count(s.unknown_data);
      g_print("after diff with default is %i, ", bitcount);
    }

    g_print("has index %i\n", data_array->len);
    g_array_append_val(data_array, s);
  }
}

/* Iterate over all sites to get all data */
alldata_t *
fill_all_data(const pip_db_t *db, const gchar *reffile, const gchar **knw) {
  alldata_t *dat = g_new(alldata_t, 1);
  bitarray_t *ref = NULL;
  unsigned udl_ref, idx = 0;
  GArray *data_array;

  data_array = g_array_new(FALSE, FALSE, sizeof(state_t));

  if (reffile)
    get_file_bindata(&ref, &udl_ref, reffile);

  while (knw[idx] != NULL) {
    const gchar *inp = knw[idx];
    process_data(data_array, inp, ref, db);
    idx++;
  }

  if (reffile)
    (void) bitarray_free(ref, FALSE);

  dat->known_data_len = BYTES_OF_BITS(db->pip_num);
  dat->unknown_data_len = udl_ref;
  dat->nstates = data_array->len;
  dat->states = (state_t *)g_array_free(data_array, FALSE);

  return dat;
}

void
free_all_data(alldata_t *dat) {
  unsigned i, nstates=dat->nstates;
  state_t *array = dat->states;

  for (i = 0; i < nstates; i++)
    release_state(&array[i]);

  g_free(array);
  g_free(dat);
}
