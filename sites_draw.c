/*
 * (C) Copyright 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
 * All rights reserved.
 */

/* Pure-drawing of the sites */
#include <math.h> /* for M_PI */
#include <cairo.h>
#include "sites.h"
#include "bitdraw.h"

#define NAME_OFFSET_X 20.0
#define NAME_OFFSET_Y 20.0
#define NAME_FONT_SITE 8.0

#define SITE_MARGIN_X 10.0
#define SITE_MARGIN_Y 10.0

#define SWITCH_CENTER_X 50.0
#define SWITCH_CENTER_Y 50.0
#define SWITCH_RADIUS   20.0

#define LUT_WIDTH  10.0
#define LUT_HEIGHT 10.0
#define LUT_BASE_X 80.0
#define LUT_BASE_Y 20.0
#define LUT_DX     00.0
#define LUT_DY     10.0

/* draw clb with no positioning */
static inline void
_draw_box(cairo_t *cr) {
  cairo_rectangle (cr, SITE_MARGIN_X, SITE_MARGIN_Y,
		   SITE_WIDTH - 2 * SITE_MARGIN_X,
		   SITE_HEIGHT - 2 * SITE_MARGIN_Y);
  cairo_stroke (cr);
}

static inline void
_draw_switchbox(cairo_t *cr) {
  /* state is ? */
  cairo_arc (cr, SWITCH_CENTER_X, SWITCH_CENTER_Y, SWITCH_RADIUS, 0, 2 * M_PI);
  cairo_stroke (cr);
}

static inline void
_draw_luts(cairo_t *cr) {
  unsigned i;
  double x = LUT_BASE_X, y = LUT_BASE_Y;
  for (i = 0; i < 4; i++) {
    x += LUT_DX;
    y += LUT_DY;
    cairo_rectangle (cr, x, y, LUT_WIDTH, LUT_HEIGHT);
    cairo_stroke (cr);
  }
}

static inline void
_draw_name(cairo_t *cr, csite_descr_t *site) {
  gchar *name = print_csite(site);
  cairo_move_to(cr, NAME_OFFSET_X, NAME_OFFSET_Y);
  cairo_show_text(cr, name);
  cairo_stroke(cr);
  g_free(name);
}

static void
_draw_clb(drawing_context_t *ctx, csite_descr_t *site) {
  cairo_t *cr = ctx->cr;
  /* cairo_append_path ? */
  _draw_box(cr);
  _draw_switchbox(cr);
  _draw_luts(cr);

  /* if text is enabled */
  if (ctx->text)
    _draw_name(cr, site);
}

/* draw the CLB with absolute positioning */
typedef void (*site_draw_t)(drawing_context_t *ctx,
			    unsigned x, unsigned y,
			    csite_descr_t *);

static void
draw_clb(drawing_context_t *ctx,
	 unsigned x, unsigned y,
	 csite_descr_t *site) {
  cairo_t *cr = ctx->cr;
  /* can be computed incrementally with only one addition */
  double dx = x * SITE_WIDTH, dy = y * SITE_HEIGHT;
  /* move to the right place */
  cairo_translate(cr, dx, dy);
  //  g_print("drawing clb %f, %f", dx, dy);
  _draw_clb(ctx, site);
  /* remove the transformation */
  cairo_translate(cr, -dx, -dy);
}

/* Drawing of regular LUT */
site_draw_t draw_table[NR_SITE_TYPE] = {
  [CLB] = draw_clb,
};

void
draw_site(unsigned x, unsigned y,
	  csite_descr_t *site, gpointer data) {
  drawing_context_t *ctx = data;
  site_draw_t fun = draw_table[site->type];
  if (fun)
    fun(ctx, x, y, site);
}

/* Drawing of the whole bunch */
static void
draw_chip(drawing_context_t *ctx, chip_descr_t *chip) {
  cairo_t *cr = ctx->cr;

  cairo_rectangle(cr, 0., 0.,
		  chip->width * SITE_WIDTH,
		  chip->height * SITE_HEIGHT);
  cairo_clip (cr);

  cairo_set_source_rgb (cr, 0., 0., 0.);
  /* paint the clip region */
  cairo_paint (cr);

  /* draw everything in white. This could be per-site or per-site-type */
  cairo_set_source_rgb (cr, 1.0, 1.0, 1.0);
  cairo_set_line_width (cr, .1);

  cairo_select_font_face(cr, "bitstream vera sans mono",
			 CAIRO_FONT_SLANT_NORMAL,
			 CAIRO_FONT_WEIGHT_NORMAL);
  cairo_set_font_size(cr, NAME_FONT_SITE);

  iterate_over_sites(chip, draw_site, ctx);
}

/* exported functions do a bunch of initialization */

/* First, initialize the structure to default value. cairo_t is passed
   from above as we are surface-agnostic in this file (rendering to pdf
   or to screen)
*/

static void
init_drawing_context(drawing_context_t *ctx, const cairo_t *cr) {
  ctx->cr = (void *)cr;
  ctx->text = TRUE;
}

drawing_context_t *
create_drawing_context(const cairo_t *cr) {
  drawing_context_t *ctx = g_new(drawing_context_t, 1);
  init_drawing_context(ctx, cr);
  return ctx;
}

cairo_t *
destroy_drawing_context(drawing_context_t *ctx) {
  cairo_t *ret = ctx->cr;
  g_free(ctx);
  return ret;
}

/*
   Then callbacks for redrawing etc, as needed for the windowing
   environment. For now pdf-only, so that's it
*/
void
draw_surface_chip(chip_descr_t *chip, cairo_surface_t *sr) {
  cairo_t *cr;
  drawing_context_t ctx;
  cr = cairo_create(sr);
  init_drawing_context(&ctx, cr);

  draw_chip(&ctx, chip);

  cairo_surface_flush(sr);
  cairo_show_page(cr);

  cairo_destroy(cr);
}

