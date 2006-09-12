/*
 * (C) Copyright 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
 * All rights reserved.
 */

#include <cairo.h>
#include <cairo-pdf.h>
#include "sites.h"
#include "bitdraw.h"

static void
draw_pdf_chip(chip_descr_t *chip) {
  cairo_surface_t *sr;

  /* extract size from chip size */
  sr = cairo_pdf_surface_create ("bitfile.pdf",
				 chip->width * SITE_WIDTH,
				 chip->height * SITE_HEIGHT);
  cairo_surface_set_fallback_resolution (sr, 600, 600);

  draw_surface_chip(chip, sr);

  cairo_surface_destroy(sr);
}

/* main, test function */

/* small testing utility */
int main() {
  chip_descr_t *chip = get_chip("/home/jb/chip/","xc2v2000");
  if (!chip)
    return -1;

  /* stdout dump */
  print_chip(chip);

  /* cairo thingy */
  draw_pdf_chip(chip);

  release_chip(chip);
  return 0;
}
