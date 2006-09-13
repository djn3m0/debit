/*
 * (C) Copyright 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
 * All rights reserved.
 */

#include <glib.h>
#include <glib/gprintf.h>

#include "wiring.h"
#include "localpips.h"
#include "bitstream.h"
#include "connexity.h"
#include "analysis.h"

/*
 * This file centralizes the work on bitstream analysis. It gets the
 * sites descriptions for the chip, then the pips for all chip sites,
 * then does the connexity analysis required to get all nets in the
 * FPGA. From here specialized modules can do more in-depth work, such
 * as VHDL/Verilog dumps.
 */

/*
 * Very simple analysis function which only dumps the pips to stdout
 */

static inline void
print_pip(const csite_descr_t *site, const gchar *start, const gchar *end) {
  gchar site_buf[32];
  sprint_csite(site_buf, site);
  g_printf("pip %s %s -> %s\n", site_buf, start, end);
}

static inline void
print_pip_iter(gpointer data,
	       wire_atom_t start, wire_atom_t end,
	       site_ref_t site) {
  pip_db_t *pipdb = data;
  wire_db_t *wiredb = pipdb->wiredb;
  gchar site_buf[32];
  sprint_csite(site_buf, site);
  g_printf("pip %s %s -> %s\n", site_buf,
	   wire_name(wiredb,start), wire_name(wiredb,end));
}

void
print_bram_data(const csite_descr_t *site, const guint16 *data) {
  guint i,j;
  g_printf("BRAM_%02x_%02x\n",
	   site->type_coord.x,
	   site->type_coord.y);
  for (i = 0; i < 64; i++) {
    g_printf("INIT_%02x:",i);
    for (j = 0; j < 16; j++)
      g_printf("%04x", data[16*i + 15 - j]);
    g_printf("\n");
  }
}

void
print_lut_data(const csite_descr_t *site, const guint16 data[]) {
  guint i;
  g_printf("CLB_%02x_%02x\n",
	   site->type_coord.x,
	   site->type_coord.y);
  for (i = 0; i < 4; i++)
    g_printf("LUT%01x:%04x\n",i,data[i]);
}

#define X_SITES 48
#define Y_SITES 56

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))
static const site_type_t types[] = {
  CLB, TTERM, BTERM, TIOI, BIOI, LIOI, RIOI, LTERM, RTERM,
  BTERMBRAM, TIOIBRAM, BIOIBRAM, BRAM,
};

static const guint xsize[SITE_TYPE_NEUTRAL] = {
  [CLB] = X_SITES,
  [TTERM] = X_SITES,
  [BTERM] = X_SITES,
  [TIOI] = X_SITES,
  [BIOI] = X_SITES,
  [LIOI] = 1,
  [RIOI] = 1,
  [RTERM] = 1,
  [LTERM] = 1,
  [TTERMBRAM] = 4,
  [BTERMBRAM] = 4,
  [TIOIBRAM] = 4,
  [BIOIBRAM] = 4,
  [BRAM] = 4,
};

static const guint ysize[SITE_TYPE_NEUTRAL] = {
  [CLB] = Y_SITES,
  [TTERM] = 1,
  [BTERM] = 1,
  [TIOI] = 1,
  [BIOI] = 1,
  [LIOI] = Y_SITES,
  [RIOI] = Y_SITES,
  [RTERM] = Y_SITES,
  [LTERM] = Y_SITES,
  [TTERMBRAM] = 1,
  [BTERMBRAM] = 1,
  [TIOIBRAM] = 1,
  [BIOIBRAM] = 1,
  [BRAM] = Y_SITES,
};

static void print_site_db(const wire_db_t *wiredb,
			  const csite_descr_t *site,
			  const pip_t *pips,
			  const gsize size) {
  gsize i;
  for (i = 0; i < size; i++ ) {
    pip_t pip = pips[i];
    print_pip(site,
	      wire_name(wiredb,pip.source),
	      wire_name(wiredb,pip.target));
  }
}

/*static*/ void
print_all_pips(pip_db_t *pipdb,
	       const bitstream_parsed_t *bitstream) {
  guint type_ref;
  guint x, y;

  /* pips */
  for (type_ref = 0; type_ref < ARRAY_SIZE(types); type_ref++) {
    site_type_t type = types[type_ref];
    for(y = 0; y < ysize[type]; y++) {
      for(x = 0; x < xsize[type]; x++) {
	pip_t *pips;
	gsize size;
	csite_descr_t site = {
	  .type_coord = { .x = x, .y = y },
	  .type = type,
	};
	pips = pips_of_site(pipdb, bitstream, &site, &size);
	print_site_db(pipdb->wiredb, &site, pips, size);
	g_free(pips);
      }
    }
  }
}

/*static*/ void
print_all_pips_direct(pip_db_t *pipdb,
		      const bitstream_parsed_t *bitstream) {
  guint type_ref;
  guint x, y;

  /* pips */
  for (type_ref = 0; type_ref < ARRAY_SIZE(types); type_ref++) {
    site_type_t type = types[type_ref];
    for(y = 0; y < ysize[type]; y++) {
      for(x = 0; x < xsize[type]; x++) {
	csite_descr_t site = {
	  .type_coord = { .x = x, .y = y },
	  .type = type,
	};
	iterate_over_bitpips(pipdb, bitstream, &site,
			     print_pip_iter, pipdb);
      }
    }
  }
}

static void
print_all_luts(const bitstream_parsed_t *bitstream) {
  guint x, y;

  for(y = 0; y < ysize[CLB]; y++) {
    for(x = 0; x < xsize[CLB]; x++) {
      guint16 luts[4];
      csite_descr_t site = {
	.type_coord = { .x = x, .y = y },
	.type = CLB,
      };
      query_bitstream_luts(bitstream, &site, luts);
      print_lut_data(&site,luts);
    }
  }
}

static void
print_all_bram(const bitstream_parsed_t *bitstream) {
  guint x, y;

  for (x = 0; x < 4; x++)
    for (y = 0; y < 14; y++) {
      guint16 *bram;
      csite_descr_t site = {
	.type_coord = { .x = x, .y = y },
      };
      bram = query_bitstream_bram_data(bitstream, &site);
      print_bram_data(&site,bram);
      g_warning("Did BRAM %i x %i", x, y);
      g_free(bram);
    }
}

/** \brief Test function which dumps the pips of a bitstream on stdout
 *
 * @param pipdb the pip database
 * @param bitstream the bitstream data
 *
 */

static void dump_all_pips(pip_db_t *pipdb,
			  const bitstream_parsed_t *bitstream) {
  print_all_bram(bitstream);
  print_all_luts(bitstream);
  print_all_pips_direct(pipdb, bitstream);
}

void dump_pips(bitstream_analyzed_t *bitstream) {
  dump_all_pips(bitstream->pipdb, bitstream->bitstream);
}

/*
 * Allocation / unallocation functions
 * Maybe split this into analysis.c
 */
static void
unfill_analysis(bitstream_analyzed_t *anal) {
  pip_db_t *pipdb = anal->pipdb;
  chip_descr_t *chip = anal->chip;

  if (pipdb)
    free_pipdb(pipdb);
  if (chip)
    release_chip(chip);
}

void
free_analysis(bitstream_analyzed_t *anal) {
  unfill_analysis(anal);
  g_free(anal);
}

static int
fill_analysis(bitstream_analyzed_t *anal,
	      bitstream_parsed_t *bitstream,
	      const gchar *datadir) {
  pip_db_t *pipdb;
  chip_descr_t *chip;

  anal->bitstream = bitstream;
  /* then fetch the databases */
  /* XXX */
  pipdb = get_pipdb(datadir);
  if (!pipdb)
    goto err_out;
  anal->pipdb = pipdb;

  /* XXX */
  chip = get_chip("/home/jb/chip/", "xc2v2000");
  if (!chip)
    goto err_out;
  anal->chip = chip;

  return 0;

 err_out:
  unfill_analysis(anal);
  return -1;
}

bitstream_analyzed_t *
analyze_bitstream(bitstream_parsed_t *bitstream,
		  const gchar *datadir) {
  bitstream_analyzed_t *anal = g_new0(bitstream_analyzed_t, 1);
  int err;

  err = fill_analysis(anal, bitstream, datadir);
  if (err) {
    g_free(anal);
    return NULL;
  }

  /* Then do some work */
  (void) build_nets(anal->pipdb, anal->chip, anal->bitstream);

  return anal;
}
