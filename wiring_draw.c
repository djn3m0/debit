/*
 * (C) Copyright 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
 * All rights reserved.
 */

#include <math.h> /* for M_PI */
#include <cairo.h>
#include "wiring.h"
#include "localpips.h"
#include "bitdraw.h"
#include "analysis.h"

static inline void
compute_wire_pos(double *x, double *y,
		 unsigned dbsize,
		 wire_atom_t wire) {
  /*
    get the wire simple. Position the thing on the circle, according to
    some ordering pre-defined. For now, be dumb enough.
    These should be memoized, not computed on the fly.
  */
  /* need the ctx. For now let's say there are 10000 wires,
     so that angle is 2 * M_PI / 10000 * wire*/
  double angle = 2 * M_PI * wire / dbsize;
  *x = (SWITCH_CENTER_X + SWITCH_RADIUS * cos(angle));
  *y = (SWITCH_CENTER_Y + SWITCH_RADIUS * sin(angle));
}

static inline void
compute_wire_endpoint(double *x, double *y,
		      const wire_db_t *wdb,
		      wire_atom_t wire) {
  /* get the wire simple */
  const wire_simple_t *simple = wire_val(wdb, wire);
  compute_wire_pos(x, y, wdb->dblen, simple->ep);
  /* translate to reach the endpoint */
  *x += -simple->dx * SITE_HEIGHT;
  *y += -simple->dy * SITE_WIDTH;
}

typedef struct _color {
  double r;
  double g;
  double b;
} color_t;

static const
color_t wire_colors[NR_WIRE_TYPE] = {
  [DOUBLE] = {0.55,1,0.6},
  [HEX] = {0.2,0.6,0.2},
  [OMUX] = {0.8,0,0},
  [WIRE_TYPE_NEUTRAL] = {0.55,0.8,0.8},
  /* rest is black */
};

static inline void
_draw_wire(const drawing_context_t *ctx,
	   const wire_db_t *wdb,
	   const wire_atom_t wire) {
  cairo_t *cr = ctx->cr;
  const wire_t *cwire = get_wire(wdb, wire);
  const color_t *color = &wire_colors[cwire->type];
  double x = 0.0, y=0.0;

  compute_wire_pos(&x, &y, wdb->dblen, wire);

  cairo_set_source_rgb (cr, color->r, color->g, color->b);
  cairo_move_to (cr, x, y);
  compute_wire_endpoint (&x, &y, wdb, wire);
  cairo_line_to (cr, x, y);
  cairo_stroke (cr);
}

cairo_pattern_t *
draw_wire_pattern(const drawing_context_t *ctx,
		  const wire_db_t *wdb,
		  const wire_atom_t wire) {
  cairo_t *cr = ctx->cr;
#define MAX_LEN 6
#define MAX_WIDTH (2*MAX_LEN + 1)
  const double zoom = ctx->zoom;
  const unsigned width = MAX_WIDTH * SITE_WIDTH * zoom, height = MAX_WIDTH * SITE_HEIGHT * zoom;
  const double dx = MAX_LEN * SITE_WIDTH, dy = MAX_LEN * SITE_HEIGHT;
  cairo_pattern_t *pat;

  cairo_save (cr);

  cairo_set_source_rgb (cr, 0., 0., 0.);
  cairo_rectangle (cr, 0, 0, width, height);
  cairo_clip (cr);
  cairo_scale (cr, zoom, zoom);

  cairo_push_group (cr);

  /* translate to the middle site for drawing */
  cairo_translate(cr, dx, dy);
  _draw_wire (ctx, wdb, wire);
  cairo_translate(cr, -dx, -dy);

  pat = cairo_pop_group (cr);

  cairo_restore (cr);

  return pat;
}

void
draw_wire_buffered(const drawing_context_t *ctx, const wire_db_t *wdb, wire_atom_t wire) {
  cairo_pattern_t *pat;
  /* compose the pattern if need be */
  pat = draw_wire_pattern(ctx, wdb, wire);
  /* compose the pattern on the surface */
  /* compose_wire_pattern(ctx, pat); */
}

static inline void
_draw_interconnect(const drawing_context_t *ctx,
		   const wire_db_t *wdb,
		   pip_t pip) {
  cairo_t *cr = ctx->cr;
  double x = 0.0, y = 0.0;

  /* Choose a color */
  cairo_set_source_rgb (cr, 1, 0.8, 0);
  compute_wire_pos(&x, &y, wdb->dblen, pip.source);
  cairo_move_to (cr, x, y);
  compute_wire_pos(&x, &y, wdb->dblen, pip.target);
  cairo_line_to (cr, x, y);
  cairo_stroke (cr);
}

static void
_draw_pip(const drawing_context_t *ctx, const wire_db_t *wdb, pip_t pip) {
  _draw_interconnect(ctx, wdb, pip);
  /* The source is not always needed --
     sometimes the pip is connected directly by the
     local interconnect. This should be given by the connexity
     analysis. */
  _draw_wire(ctx, wdb, pip.source);
  /* _draw_wire(ctx, wdb, pip.target); */
}

/* so now let's talk good */
typedef struct _wire_iter {
  const drawing_context_t *ctx;
  const wire_db_t *wdb;
  const chip_descr_t *chip;
} wire_iter_t;

static void
draw_wire_iter(gpointer data,
	       wire_atom_t start, wire_atom_t end,
	       site_ref_t site) {
  wire_iter_t *iter = data;
  pip_t pip = { .source = start,
		.target = end, };
  cairo_t *cr = iter->ctx->cr;
  const chip_descr_t *chip = iter->chip;
  unsigned width = chip->width;
  unsigned index = site_index(site);
  double dx = (index % width) * SITE_WIDTH, dy = (index / width) * SITE_HEIGHT;

  /* These save / restore could be balanced so that we don't do them too
     many times, which is currently the case */
  cairo_save (cr);

  cairo_translate (cr, dx, dy);
  _draw_pip (iter->ctx, iter->wdb, pip);

  cairo_restore (cr);
}

/* \brief Draw all pips in a bitstream
 *
 * Until the connexity analysis is completed, this drawing is really
 * dumb. This is to be able to compile the net-drawn wires.
 *
 */

void
draw_all_wires(drawing_context_t *ctx,
	       const bitstream_analyzed_t *nlz) {
  cairo_t *cr = ctx->cr;
  const double zoom = ctx->zoom;
  const chip_descr_t *chip = nlz->chip;
  const pip_parsed_dense_t *pipdat = nlz->pipdat;
  wire_iter_t iter = { .ctx = ctx,
		       .chip = nlz->chip,
		       .wdb = nlz->pipdb->wiredb, };

  cairo_set_line_width (ctx->cr, 1.0);

  cairo_save (cr);

  cairo_scale (cr, zoom, zoom);

  cairo_translate (cr, -ctx->x_offset, -ctx->y_offset);
  iterate_over_bitpips(pipdat, chip, draw_wire_iter, &iter);

  cairo_restore (cr);
}

/*
 * More complex, duplicated implementation with range limitation
 */

typedef struct _wire_iter_limited {
  const drawing_context_t *ctx;
  const wire_db_t *wdb;
  const chip_descr_t *chip;
  const site_area_t *area;
  unsigned drawn_pips;
} wire_iter_limited_t;

static void
draw_wire_iter_limited(gpointer data,
		       wire_atom_t start, wire_atom_t end,
		       site_ref_t site) {
  wire_iter_limited_t *iter = data;
  pip_t pip = { .source = start,
		.target = end, };
  cairo_t *cr = iter->ctx->cr;
  const chip_descr_t *chip = iter->chip;
  const site_area_t *area = iter->area;
  unsigned width = chip->width;
  unsigned index = site_index(site);
  double dx = (index % width) * SITE_WIDTH, dy = (index / width) * SITE_HEIGHT;

  /* if we're not in range, skip it */
  if (index % width - area->x > area->width ||
      index / width - area->y > area->height)
    return;

  /* These save / restore could be balanced so that we don't do them too
     many times, which is currently the case */
  cairo_save (cr);

  cairo_translate (cr, dx, dy);
  _draw_pip (iter->ctx, iter->wdb, pip);
  iter->drawn_pips++;

  cairo_restore (cr);

}

void
draw_all_wires_limited(drawing_context_t *ctx,
		       const bitstream_analyzed_t *nlz,
		       const site_area_t *area) {
  cairo_t *cr = ctx->cr;
  const double zoom = ctx->zoom;
  const chip_descr_t *chip = nlz->chip;
  const pip_parsed_dense_t *pipdat = nlz->pipdat;
  wire_iter_limited_t iter = { .ctx = ctx,
			       .chip = nlz->chip,
			       .wdb = nlz->pipdb->wiredb,
			       .area = area,
			       .drawn_pips = 0,
  };

  cairo_set_line_width (ctx->cr, 1.0);

  cairo_save (cr);

  cairo_scale (cr, zoom, zoom);

  cairo_translate (cr, -ctx->x_offset, -ctx->y_offset);

  iterate_over_bitpips(pipdat, chip, draw_wire_iter_limited, &iter);
  g_print("%i pips drawn\n",iter.drawn_pips);

  cairo_restore (cr);
}

void
draw_cairo_wires(cairo_t *cr, const bitstream_analyzed_t *nlz) {
  drawing_context_t ctx;

  init_drawing_context(&ctx);
  set_cairo_context(&ctx, cr);
  draw_all_wires(&ctx, nlz);
}

/* \brief Draw all pips in a bitstream, by nets
 *
 * More intelligent way of drawing things, somewhat structured
 */

void draw_wires_by_net(const drawing_context_t *ctx) {
  // iterate_over_nets
  (void) ctx;
  return;
}

/* iterate over the pips in the thing */
/* void draw_all_things(const drawing_context_t *ctx, */
/* 		     const site_details_t *site) { */
/*   const pip_db_t *pipdb = ctx->pipdb; */
/*   const bitstream_parsed_t *bitstream = ctx->bitstream; */
/*   /\* dynamical, whoo-hoo ! Actually should be done after connexity */
/*      analysis *\/ */
/*   gsize size, i; */
/*   pip_t *pips; */

/*   pips = pips_of_site(pipdb, bitstream, site, &size); */

/*   /\* for all of these *\/ */
/*   for(i = 0; i < size; i++) */
/*     draw_pip(ctx, pips[i]); */

/*   g_free(pips); */
/* } */
