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

static int
_parse_bitfile(bitstream_parsed_t *dest,
	       const char *buf_in, const off_t len) {
  int offset;
  bitstream_parser_t parser;

  /* memset the parser */
  memset(&parser, 0, sizeof(parser));

  /* parse the header */
  offset = parse_header(&dest->header, buf_in, len);

  if (offset < 0) {
    debit_log(L_BITSTREAM,"header parsing error");
    return -1;
  }

  /* Do some allocations according to the type of the bitfile */
  bytearray_init(&parser.ba, len, offset, buf_in);

  return _parse_bitstream_data(dest, &parser);
}

/* High-level functions */

bitstream_parsed_t *
parse_bitstream(const gchar*filename) {
  bitstream_parsed_t *dest;

  GError *error = NULL;
  GMappedFile *file = NULL;
  int err = 0;

  dest = g_new0(bitstream_parsed_t, 1);

  file = g_mapped_file_new (filename, FALSE, &error);
  dest->file = file;

  if (error != NULL) {
    debit_log(L_BITSTREAM,"could not map file %s: %s",filename,error->message);
    g_error_free (error);
    goto out_free_dest;
  }

  err = _parse_bitfile(dest,
		       g_mapped_file_get_contents (file),
		       g_mapped_file_get_length (file));

  if (err < 0)
    goto out_free;

  return dest;

 out_free:
  g_mapped_file_free (file);
 out_free_dest:
  g_free (dest);
  return NULL;
}

void
free_bitstream(bitstream_parsed_t *bitstream) {
  free_indexer(bitstream);
  g_mapped_file_free(bitstream->file);
  g_free(bitstream);
  return;
}
