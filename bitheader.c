/*
 * Copyright (C) 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
 */

#include <glib.h>
#include "bitheader.h"
#include "debitlog.h"

static void
parse_synchro (synchro_option_t *opt) {
  guint len = GUINT32_FROM_BE(opt->bitstream_length);
  debit_log(L_HEADER, "Synchro option, bitstream length %i", len);
  (void) len;
  return;
}

int
parse_header(const gchar *buf, const size_t buf_len) {
  header_option_t *current;
  synchro_option_t *opt;
  const gchar *start = buf;
  const gchar *end;

  /* seek the first option -- I need bitstream examples to
     reverse-engeneer the very first option */
  while (*buf != FILENAME) {
    buf++;
    if (buf > start + buf_len)
      return -1;
  }

  current = (void *) buf;

  while (current->code != CODE) {
    /* parse_option(current_option); */
    current = next_option(current);
  }

  opt = (void*) current;
  parse_synchro(opt);
  end = (void *) opt->data;
  //end = (void *) next_option(current);
  return (end - start);
}
