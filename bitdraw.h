/*
 * (C) Copyright 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
 * All rights reserved.
 */

#ifndef _HAS_BITDRAW_H
#define _HAS_BITDRAW_H

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

//void draw_chip(drawing_context_t *ctx, chip_descr_t *chip);
void draw_chip_monitored(drawing_context_t *ctx, chip_descr_t *chip);

/* draw onto surface the chip */
void draw_surface_chip(chip_descr_t *chip, cairo_surface_t *sr);

/* wire drawing primitives */
void draw_wire(const drawing_context_t *ctx, wire_atom_t wire);
void draw_interconnect(const drawing_context_t *ctx, pip_t pip);
void draw_pip(const drawing_context_t *ctx, pip_t pip);

/* bad, this. The drawing context should be separated from the cairo_t */
drawing_context_t *drawing_context_create();
void drawing_context_destroy(drawing_context_t *ctx);

static inline
void set_cairo_context(drawing_context_t *ctx, cairo_t *cr) {
  ctx->cr = cr;
}

#endif /* _HAS_BITDRAW_H */
