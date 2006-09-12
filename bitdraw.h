/*
 * (C) Copyright 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
 * All rights reserved.
 */

#include <cairo.h>
#include <glib.h>
#include "sites.h"
#include "wiring.h"

typedef struct _drawing_context {
  cairo_t *cr;
  gboolean text;

  /* structural information */
  chip_descr_t *chip;
  wire_db_t *wiredb;
} drawing_context_t;

/* All sites have the same width and height. Simpler. */
#define SITE_WIDTH 100.0
#define SITE_HEIGHT 100.0

drawing_context_t *
create_drawing_context(const cairo_t *cr);

cairo_t *
destroy_drawing_context(drawing_context_t *ctx);

/* draw onto surface the chip.
   This is bad, as it does a host of uneeded initializations which
   should be isolated and not redone for redraws */
void draw_surface_chip(chip_descr_t *chip, cairo_surface_t *sr);

/* wire drawing primitives */
void draw_wire(const drawing_context_t *ctx, wire_atom_t wire);
void draw_interconnect(const drawing_context_t *ctx, pip_t pip);
void draw_pip(const drawing_context_t *ctx, pip_t pip);
