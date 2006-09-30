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

static void
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

static void
print_lut_data(const csite_descr_t *site, const guint16 data[]) {
  guint i;
  g_printf("CLB_%02x_%02x\n",
	   site->type_coord.x,
	   site->type_coord.y);
  for (i = 0; i < 4; i++)
    g_printf("LUT%01x:%04x\n",i,data[i]);
}

static void
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

static void
print_all_pips(const pip_db_t *pipdb, const chip_descr_t *chip,
	       const pip_parsed_dense_t *pipdat) {
  iterate_over_bitpips(pipdat, chip, print_pip_iter, (gpointer)pipdb);
}

static void
print_lut_iter(unsigned site_x, unsigned site_y,
	       csite_descr_t *site, gpointer dat) {
  bitstream_parsed_t *bitstream = dat;
  guint16 luts[4];
  query_bitstream_luts(bitstream, site, luts);
  print_lut_data(site,luts);
}

static void
print_all_luts(const chip_descr_t *chip,
	       const bitstream_parsed_t *bitstream) {
  iterate_over_typed_sites(chip, CLB, print_lut_iter, (gpointer)bitstream);
}

static void
print_bram_iter(unsigned site_x, unsigned site_y,
		csite_descr_t *site, gpointer dat) {
  bitstream_parsed_t *bitstream = dat;
  guint16 *bram;
  bram = query_bitstream_bram_data(bitstream, site);
  print_bram_data(site,bram);
  g_warning("Did BRAM %i x %i", site_x, site_y);
  g_free(bram);
}

static void
print_all_bram(const chip_descr_t *chip,
	       const bitstream_parsed_t *bitstream) {
  iterate_over_typed_sites(chip, BRAM, print_bram_iter, (gpointer)bitstream);
}

/** \brief Test function which dumps the pips of a bitstream on stdout
 *
 * @param pipdb the pip database
 * @param bitstream the bitstream data
 *
 */

static void dump_all_pips(const pip_db_t *pipdb, const chip_descr_t *chipdb,
			  const pip_parsed_dense_t *pipdat,
			  const bitstream_parsed_t *bitstream) {
  print_all_bram(chipdb, bitstream);
  print_all_luts(chipdb, bitstream);
  print_all_pips(pipdb, chipdb, pipdat);
}

void dump_pips(bitstream_analyzed_t *bitstream) {
  dump_all_pips(bitstream->pipdb, bitstream->chip, bitstream->pipdat, bitstream->bitstream);
}

/*
 * Allocation / unallocation functions
 * Maybe split this into analysis.c
 */
static void
unfill_analysis(bitstream_analyzed_t *anal) {
  pip_db_t *pipdb = anal->pipdb;
  chip_descr_t *chip = anal->chip;
  pip_parsed_dense_t *pipdat = anal->pipdat;

  if (pipdb)
    free_pipdb(pipdb);
  if (chip)
    release_chip(chip);
  if (pipdat)
    free_pipdat(pipdat);
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
  pip_parsed_dense_t *pipdat;

  anal->bitstream = bitstream;
  /* then fetch the databases */
  pipdb = get_pipdb(datadir);
  if (!pipdb)
    goto err_out;
  anal->pipdb = pipdb;

  chip = get_chip(datadir, bitstream->chip);
  if (!chip)
    goto err_out;
  anal->chip = chip;

  pipdat = pips_of_bitstream(pipdb, chip, bitstream);
  if (!pipdat)
    goto err_out;
  anal->pipdat = pipdat;

  return 0;

 err_out:
  unfill_analysis(anal);
  return -1;
}

bitstream_analyzed_t *
analyze_bitstream(bitstream_parsed_t *bitstream,
		  const gchar *datadir) {
  bitstream_analyzed_t *anal = g_new0(bitstream_analyzed_t, 1);
/*   nets_t * nets; */
  int err;

  err = fill_analysis(anal, bitstream, datadir);
  if (err) {
    g_free(anal);
    return NULL;
  }

  /* Then do some work */
/*   nets = build_nets(anal->pipdb, anal->chip, anal->pipdat); */
/*   print_nets(nets, anal->pipdb, anal->chip); */
/*   free_nets(nets); */

  return anal;
}
