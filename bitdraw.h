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

#ifndef _HAS_BITDRAW_H
#define _HAS_BITDRAW_H

#include <cairo.h>
#include <glib.h>
#include "sites.h"
#include "wiring.h"
#include "localpips.h"
#include "bitstream_parser.h"
#include "analysis.h"

#define SWITCH_CENTER_X 50.0
#define SWITCH_CENTER_Y 50.0
#define SWITCH_RADIUS   15.0

typedef struct _drawing_context {
  cairo_t *cr;
  gboolean text;
  /* cairo information */
  cairo_pattern_t *site_sing_patterns[NR_SITE_TYPE];
  cairo_pattern_t *site_line_patterns[NR_SITE_TYPE];
  cairo_pattern_t *site_full_patterns[NR_SITE_TYPE];

  /* And I also want lazy patterns for wires */
  cairo_pattern_t **pip_patterns;

  /* Drawing parameters */
  gint x_offset;
  gint y_offset;
  double zoom;

  /* structural information */

} drawing_context_t;

/* Initialize the structure to default value. cairo_t is passed
   from above as we are surface-agnostic in this file (rendering to pdf
   or to screen)
*/

static inline void
init_drawing_context(drawing_context_t *ctx) {
  unsigned i;
  ctx->cr = NULL;
  ctx->text = FALSE;
  ctx->x_offset = 0;
  ctx->y_offset = 0;
  ctx->zoom = 1.0;

  for (i = 0; i < NR_SITE_TYPE; i++) {
    ctx->site_sing_patterns[i] = NULL;
    ctx->site_line_patterns[i] = NULL;
    ctx->site_full_patterns[i] = NULL;
  }
}

/* All sites have the same width and height. Simpler. */
#define SITE_WIDTH 100.0
#define SITE_HEIGHT 100.0

void generate_patterns(drawing_context_t *ctx);
void draw_chip(drawing_context_t *ctx, const chip_descr_t *chip);
void destroy_patterns(drawing_context_t *ctx);

/* draw the chip layout */
//void draw_surface_chip(cairo_surface_t *sr, const chip_descr_t *chip);
void draw_cairo_chip(cairo_t *cr, const chip_descr_t *chip);

/* draw individual site */
void _draw_site_compose(const drawing_context_t *ctx, const csite_descr_t *site);

/* wire drawing primitives -- this is where the real work happens */
void draw_all_wires(drawing_context_t *ctx, const bitstream_analyzed_t *nlz);

/* Describes a rectangular site range. Possibly merged into
 * nsite_area_t once it is completed. */
typedef struct site_area {
  unsigned x;
  unsigned width;
  unsigned y;
  unsigned height;
} site_area_t;

void draw_all_wires_limited(drawing_context_t *ctx,
			    const bitstream_analyzed_t *nlz,
			    const site_area_t *area);
void draw_cairo_wires(cairo_t *cr, const bitstream_analyzed_t *nlz);

/* bad, this. The drawing context should be separated from the cairo_t */
drawing_context_t *drawing_context_create();
void drawing_context_destroy(drawing_context_t *ctx);

static inline
void set_cairo_context(drawing_context_t *ctx, cairo_t *cr) {
  ctx->cr = cr;
}

static inline
double chip_drawing_width(const chip_descr_t *chip) {
  return chip->width * SITE_WIDTH;
}

static inline
double chip_drawing_height(const chip_descr_t *chip) {
  return chip->height * SITE_HEIGHT;
}

#endif /* _HAS_BITDRAW_H */
