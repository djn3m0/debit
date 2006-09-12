/*
 * Copyright (C) 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
 */

#include <glib.h>

static gboolean valdump = FALSE;
static gchar **files = NULL;
static gint chomp = 0;

static GOptionEntry entries[] =
{
  {"valdump", 'v', 0, G_OPTION_ARG_NONE, &valdump, "Dump value at differing positions", NULL},
  {"chomp", 'c', 0, G_OPTION_ARG_INT, &chomp, "Chomp thus many bytes from the end of the comparison zone", NULL},
  { G_OPTION_REMAINING, 'r', 0, G_OPTION_ARG_FILENAME_ARRAY, &files, "Files to compare", NULL},
  { NULL }
};

static int
compare_data(const gchar *dat1, const unsigned l1,
	     const gchar *dat2, const unsigned l2) {
  unsigned i;
  unsigned long differences = 0;

  if (l1 != l2) {
    g_warning("File sizes differ");
    return -1;
  }

  for (i = 0; i < l1 - chomp; i++) {
    gchar val1, val2;
    val1 = dat1[i];
    val2 = dat2[i];

    if (val1 != val2) {
      unsigned j;
      for (j = 0; j < 8; j++) {
	gchar mask = 1 << j;
	unsigned bit1 = val1 & mask, bit2 = val2 & mask;
	if (bit1 != bit2) {
	  unsigned pos = 8*i+j;
	  differences += 1;
	  if (!valdump)
	    g_print("%i\n", pos);
	  else
	    g_print("%i %i %i\n", pos, bit1 ? 1 : 0, bit2 ? 1 : 0);
	}
      }
    }
  }

  return (int) differences;

}

static int
compare_files(const gchar *f1, const gchar *f2) {
  GError *error = NULL;
  GMappedFile *file1 = NULL, *file2 = NULL;
  int err;

  file1 = g_mapped_file_new (f1, FALSE, &error);
  if (error != NULL) {
    file1 = NULL;
    err = -1;
    goto out_err;
  }

  file2 = g_mapped_file_new (f2, FALSE, &error);
  if (error != NULL) {
    file2 = NULL;
    err = -1;
    goto out_err;
  }

  /* Then update */
  err = compare_data(g_mapped_file_get_contents (file1),
		     g_mapped_file_get_length (file1),
		     g_mapped_file_get_contents (file2),
		     g_mapped_file_get_length (file2));

  goto out;

  out_err:
    g_warning ("error opening file: %s",error->message);
    g_error_free (error);
  out:
    if (file1)
      g_mapped_file_free (file1);
    if (file2)
      g_mapped_file_free (file2);
    return err;
}

int
main(int argc, char *argv[])
{
  GError *error = NULL;
  GOptionContext *context = NULL;

  context = g_option_context_new ("- bitwise compare of files");
  g_option_context_add_main_entries (context, entries, NULL);
  g_option_context_parse (context, &argc, &argv, &error);
  if (error != NULL) {
    g_warning("parse error: %s",error->message);
    g_error_free (error);
    return -1;
  }

  g_option_context_free(context);

  if (!files) {
	  g_warning("parse error: you must specify input files");
	  return -1;
  }

  /* Get the remaining arguments */
  return compare_files(files[0], files[1]);
}
