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
  debit_log(L_HEADER, "bitstream data starting @%p", opt->data);
  (void) len;
  return;
}

static void
parse_option (parsed_header_t *parse,
	      const header_option_t *opt) {

  option_type_t code = opt->code;
  gint len = get_option_len(opt);

  debit_log(L_HEADER, "Option code %i, length %i", code, len);
  debit_log(L_HEADER, "data: %.*s",len,opt->payload);

  /* If in range, then record the option */
  if (code < LAST_OPTION) {
    parse->options[code - FILENAME].len = len;
    parse->options[code - FILENAME].data = opt->payload;
  }
  else
    debit_log(L_HEADER, "Option code unknown, please report");

}

int
parse_header(parsed_header_t *parse,
	     const gchar *buf, const size_t buf_len) {
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
    parse_option(parse, current);
    current = next_option(current);
  }

  opt = (void*) current;
  parse_synchro(opt);
  end = (void *) opt->data;

  return (end - start);
}
