/*
 * (C) Copyright 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
 * All rights reserved.
 */

#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdint.h>

#include <glib.h>

#include "bitstream_parser.h"

/* XXX This is currently needed but should be removed */
#include "design.h"

typedef void (*dump_hook_t)(FILE *out, const void *_data, const unsigned num);
static void dump_bin_rev(FILE *out, const void *_data, const unsigned num);

/**
 * Virtex-II dump function
 */

static void
dump_bin_rev(FILE *out, const void *_data, const unsigned num) {
  const unsigned char *const data = _data;
  unsigned i;
  for( i = 0; i < num; i++) {
    unsigned char atom = data[num-1-i];
    putc(atom, out);
  }
}

static inline void
dump_32be(FILE *out, const uint32_t dat) {
  unsigned i;
  /* dump le first */
  for( i = 0; i < 4; i++) {
    unsigned char atom;
    atom = (dat >> (8 * (3-i))) & 0xff;
    putc(atom, out);
  }
}

static void
dump_bin(FILE *out, const void *_data, const unsigned num) {
  const uint32_t *const data = _data;
  unsigned i;
  for( i = 0; i < num / sizeof(uint32_t); i++)
    dump_32be(out, GUINT32_FROM_LE(data[i]));
}

typedef void (*naming_hook_t)(char *buf, unsigned buf_len,
			      const unsigned type,
			      const unsigned index,
			      const unsigned frameid);

static FILE *open_frame_file(const unsigned type,
			     const unsigned index,
			     const unsigned frameid,
			     const gchar *odir,
			     naming_hook_t name) {
  FILE *f;
  char fn[64];
  gchar *filename = NULL;
  name(fn, sizeof(fn), type, index, frameid);
  filename = g_build_filename(odir,fn,NULL);
  f = fopen(filename, "w");
  if (!f)
    g_warning("could not open file %s", filename);
  g_free(filename);
  return f;
}

static FILE *
open_unk_frame_file(const frame_record_t *frame) {
  FILE *f;
  char fn[64];
  char farname[64];
  snprintf_far(farname, sizeof(farname), frame->far);
  snprintf(fn, sizeof(fn), "frame_%s.bin", farname);
  f = fopen(fn, "w");
  return f;
}

static void
seq_frame_name(char *buf, unsigned buf_len,
	       const unsigned type,
	       const unsigned index,
	       const unsigned frameid) {
  snprintf(buf, buf_len, "frame_%02x_%02x_%02x",
	   type, index, frameid);
}

typedef struct _dumping {
  dump_hook_t dump;
  naming_hook_t naming;
  unsigned framelen;
  const gchar *dir;
} dumping_t;

/* XXX Frame length is static */
static void
design_write_frames_iter(const char *frame,
			 guint type, guint index, guint frameidx,
			 void *data) {
  dumping_t *dumping = data;
  dump_hook_t dump = dumping->dump;
  naming_hook_t naming = dumping->naming;
  unsigned framelen = dumping->framelen;
  const gchar *odir = dumping->dir;
  FILE *f;
  gsize frame_len = framelen * sizeof(uint32_t);

  f = open_frame_file(type, index, frameidx, odir, naming);
  if (!f) {
    g_warning("could not open file for frame");
    return;
  }
  dump(f, frame, frame_len);
  fclose(f);
}

static void
design_write_unk_frames_iter(const frame_record_t *framerec,
			     void *data) {
  dumping_t *dumping = data;
  dump_hook_t dump = dumping->dump;
  FILE *f = open_unk_frame_file(framerec);
  dump(f, framerec->frame, framerec->framelen * sizeof(uint32_t));
  fclose(f);
}

/**
 * for virtex-II
 */

void design_write_frames(const bitstream_parsed_t *parsed,
			 const gchar *outdir) {
  dumping_t data;
  const chip_struct_t *chip = parsed->chip_struct;

  data.dump = dump_bin_rev;
  data.dir  = outdir;
  data.framelen = chip->framelen;

  if (TRUE)
    data.naming = typed_frame_name;
  else
    data.naming = seq_frame_name;

  iterate_over_frames(parsed, design_write_frames_iter, &data);
}

/**
 * for virtex-4 and virtex-5
 */

void
design_dump_frames(const bitstream_parsed_t *parsed,
		   const gchar *outdir) {
  dumping_t data;
  data.dump = dump_bin;
  data.naming = seq_frame_name;
  iterate_over_unk_frames(parsed, design_write_unk_frames_iter, &data);
}
