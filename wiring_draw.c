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
  /* need the ctx. For now let's say there are 10000 wires,
     so that angle is 2 * M_PI / 10000 * wire*/
  double angle = 2 * M_PI * wire / 10000.;
  *x = (SWITCH_CENTER_X + SWITCH_RADIUS * cos(angle));
  *y = (SWITCH_CENTER_X + SWITCH_RADIUS * sin(angle));
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
	   const wire_atom_t wire) {
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

cairo_pattern_t *
draw_wire_pattern(const drawing_context_t *ctx,
		  const wire_atom_t wire) {
  cairo_t *cr = ctx->cr;
#define MAX_LEN 6
#define MAX_WIDTH (2*MAX_LEN + 1)
  const double dx = MAX_LEN * SITE_WIDTH, dy = MAX_LEN * SITE_HEIGHT;
  cairo_pattern_t *pat;

  cairo_save (cr);

  /* Clip before push_group */
  cairo_rectangle (cr, 0, 0,
		   MAX_WIDTH * SITE_WIDTH,
		   MAX_WIDTH * SITE_HEIGHT);
  cairo_clip (cr);

  cairo_push_group (cr);

  /* translate to the middle site for drawing */
  cairo_translate(cr, dx, dy);
  _draw_wire (ctx, wire);
  cairo_translate(cr, -dx, -dy);

  pat = cairo_pop_group (cr);

  cairo_restore (cr);

  return pat;
}

void
draw_wire(const drawing_context_t *ctx, wire_atom_t wire) {
  cairo_t *cr = ctx->cr;
  cairo_save (cr);
  _draw_wire(ctx, wire);
  cairo_restore (cr);
}

void
draw_wire_buffered(const drawing_context_t *ctx, wire_atom_t wire) {
  /* compose the pattern if need be */

  /* compose the pattern on the surface */
}

static inline void
_draw_interconnect(const drawing_context_t *ctx,
		   pip_t pip) {
  cairo_t *cr = ctx->cr;
  double x = 0.0, y=0.0;

  /* Choose a color */
  cairo_set_source_rgb (cr, 1.0, 1.0, 0.2);
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
