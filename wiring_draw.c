/*
 * (C) Copyright 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
 * All rights reserved.
 */

#include <math.h> /* for M_PI */
#include <cairo.h>
#include "wiring.h"
#include "bitdraw.h"

static inline void
compute_wire_pos(double *x, double *y,
		 wire_atom_t wire) {
  /*
    get the wire simple. Position the thing on the circle, according to
    some ordering pre-defined. For now, be dumb enough.
    These should be memoized, not computed on the fly.
  */
}

static inline void
compute_wire_endpoint(double *x, double *y,
		      wire_atom_t wire) {
  /* get the wire simple */
  wire_simple_t *simple = NULL;
  compute_wire_pos(x, y, simple->ep);
  /* translate to reach the endpoint */
  *x += simple->dx * SITE_HEIGHT;
  *y += simple->dy * SITE_WIDTH;
}

typedef struct _color {
  double r;
  double g;
  double b;
} color_t;

color_t wire_colors[NR_WIRE_TYPE] = {
  [DOUBLE] = {0.55,1,0.6},
  [HEX] = {0.2,0.6,0.2},
  [OMUX] = {0.8,0,0},
  [WIRE_TYPE_NEUTRAL] = {1.0,1.0,1.0},
  /* rest is black */
};

static inline void
_draw_wire(const drawing_context_t *ctx,
	   wire_atom_t wire) {
  cairo_t *cr = ctx->cr;
  wire_type_t type =  WIRE_TYPE_NEUTRAL;
  color_t *color = &wire_colors[type];
  double x = 0.0, y=0.0;

  compute_wire_pos(&x, &y, wire);

  cairo_set_source_rgb (cr, color->r, color->g, color->b);
  cairo_move_to (cr, x, y);
  compute_wire_endpoint (&x, &y, wire);
  cairo_line_to (cr, x, y);
  cairo_stroke (cr);
}

void
draw_wire(const drawing_context_t *ctx, wire_atom_t wire) {
  cairo_t *cr = ctx->cr;
  cairo_save (cr);
  _draw_wire(ctx, wire);
  cairo_restore (cr);
}

static inline void
_draw_interconnect(const drawing_context_t *ctx,
		   pip_t pip) {
  cairo_t *cr = ctx->cr;
  double x = 0.0, y=0.0;

  /* Choose a color */
/*   cairo_set_source_rgb (cr, color->r, color->g, color->b); */
  compute_wire_pos(&x, &y, pip.source);
  cairo_move_to (cr, x, y);
  compute_wire_pos(&x, &y, pip.target);
  cairo_line_to (cr, x, y);
}

void
draw_interconnect(const drawing_context_t *ctx,
		  pip_t pip) {
  cairo_t *cr = ctx->cr;
  cairo_save(cr);
  _draw_interconnect(ctx, pip);
  cairo_restore(cr);
}

void
draw_pip(const drawing_context_t *ctx, pip_t pip) {
  cairo_t *cr = ctx->cr;
  cairo_save (cr);
  _draw_wire(ctx, pip.source);
  _draw_interconnect(ctx, pip);
  _draw_wire(ctx, pip.target);
  cairo_restore (cr);
}
