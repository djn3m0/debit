/*
 * (C) Copyright 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
 * All rights reserved.
 */

#include <glib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>

//#include "virtex2_config.h"
#include "bitstream_parser.h"
#include "design.h"

static const
char *type_names[V2C__NB_CFG] = {
  [V2C_IOB] = "IOB",
  [V2C_IOI] = "IOI",
  [V2C_CLB] = "CLB",
  [V2C_BRAM] = "BRAM",
  [V2C_BRAM_INT] = "BRAM_INT",
  [V2C_GCLK] = "GCLK",
};

typedef void (*dump_hook_t)(FILE *out, const void *_data, const unsigned num);
static void dump_bin(FILE *out, const void *_data, const unsigned num);

static void
dump_bin(FILE *out, const void *_data, const unsigned num) {
  const unsigned char *const data = _data;
  unsigned i;
  for( i = 0; i < num; i++) {
    unsigned char atom = data[num-1-i];
    putc(atom, out);
  }
}

typedef void (*naming_hook_t)(char *buf, unsigned buf_len,
			      const unsigned type,
			      const unsigned index,
			      const unsigned frameid);

static FILE *open_frame_file(const unsigned type,
			     const unsigned index,
			     const unsigned frameid,
			     naming_hook_t name) {
  FILE *f;
  char fn[64];
  name(fn, sizeof(fn), type, index, frameid);
  f = fopen(fn, "w");
  return f;
}

static FILE *
open_unk_frame_file(const frame_record_t *frame) {
  FILE *f;
  char fn[64];
  snprintf(fn, sizeof(fn), "frame_%i_%i.bin", frame->far, frame->offset);
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

static void
typed_frame_name(char *buf, unsigned buf_len,
		 const unsigned type,
		 const unsigned index,
		 const unsigned frameid) {
  snprintf(buf, buf_len, "frame_%s_%02x_%02x",
	   type_names[type], index, frameid);
}

typedef struct _dumping {
  dump_hook_t dump;
  naming_hook_t naming;
} dumping_t;

/* XXX Frame length is static */
static void
design_write_frames_iter(const char *frame,
			 guint type, guint index, guint frameidx,
			 void *data) {
  dumping_t *dumping = data;
  dump_hook_t dump = dumping->dump;
  naming_hook_t naming = dumping->naming;
  FILE *f;
  gsize frame_len = 146 * sizeof(uint32_t);

  f = open_frame_file(type, index, frameidx, naming);
  dump(f, frame, frame_len);
  fclose(f);
}

static void
design_write_unk_frames_iter(const frame_record_t *framerec,
			     void *data) {
  dumping_t *dumping = data;
  dump_hook_t dump = dumping->dump;
  FILE *f = open_unk_frame_file(framerec);
  dump(f, framerec->frame, framerec->framelen);
  fclose(f);
}

void design_write_frames(const bitstream_parsed_t *parsed,
			 const gchar *outdir) {
  dumping_t data;

  data.dump = dump_bin;

  if (TRUE)
    data.naming = typed_frame_name;
  else
    data.naming = seq_frame_name;

  iterate_over_frames(parsed, design_write_frames_iter, &data);
}

void
design_dump_frames(const bitstream_parsed_t *parsed,
		   const gchar *outdir) {
  dumping_t data;
  data.dump = dump_bin;
  data.naming = seq_frame_name;
  iterate_over_unk_frames(parsed, design_write_unk_frames_iter, &data);
}
