/*
 * (C) Copyright 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
 * All rights reserved.
 */

/* Pure-drawing of the sites */
#include <math.h> /* for M_PI */
#include <cairo.h>
#include "sites.h"
#include "bitdraw.h"

#include <sys/time.h>
#include <time.h>

#define NAME_OFFSET_X 20.0
#define NAME_OFFSET_Y 20.0
#define NAME_FONT_SIZE 8.0
#define NAME_FONT_TYPE "bitstream vera sans mono"

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
  //g_print("printing %s", name);
  cairo_move_to(cr, NAME_OFFSET_X, NAME_OFFSET_Y);
  cairo_show_text(cr, name);
  cairo_stroke(cr);
  g_free(name);
}

void
_draw_clb_pattern(cairo_t *cr) {
  _draw_box(cr);
  _draw_switchbox(cr);
  _draw_luts(cr);
}

static void
_draw_clb(drawing_context_t *ctx, csite_descr_t *site) {
  cairo_t *cr = ctx->cr;

  _draw_clb_pattern(cr);

  /* if text is enabled */
  if (ctx->text)
    _draw_name(cr, site);
}

cairo_pattern_t *
draw_clb_pattern(drawing_context_t *ctx) {
  cairo_t *cr = ctx->cr;
  cairo_pattern_t *pat;
  cairo_save (cr);

  cairo_rectangle (cr, 0, 0, SITE_WIDTH, SITE_HEIGHT);
  cairo_clip (cr);
  cairo_push_group (cr);
  _draw_clb_pattern (cr);
  pat = cairo_pop_group (cr);

  cairo_restore (cr);
  return pat;
}

void
_draw_clb_new(drawing_context_t *ctx, csite_descr_t *site) {
  cairo_t *cr = ctx->cr;
  cairo_pattern_t *site_pattern = ctx->site_patterns[CLB];

  /* now let's draw !
     NB: could use the *whole* thing as pattern */

  /* clip */
  cairo_save (cr);
  /* slower with the clip */
/*   cairo_rectangle (cr, 0, 0, SITE_WIDTH, SITE_HEIGHT); */
/*   cairo_clip (cr); */
  cairo_set_source (cr, site_pattern);
  cairo_paint (cr);

  cairo_restore (cr);

  /* if text is enabled */
/*   if (ctx->text) */
/*     _draw_name(cr, site); */
}

/* draw the CLB with absolute positioning */
typedef void (*site_draw_t)(drawing_context_t *ctx,
			    unsigned x, unsigned y,
			    csite_descr_t *);

void
draw_clb(drawing_context_t *ctx,
	 unsigned x, unsigned y,
	 csite_descr_t *site) {
  cairo_t *cr = ctx->cr;
  /* can be computed incrementally with only one addition */
  double dx = x * SITE_WIDTH, dy = y * SITE_HEIGHT;
  /* move to the right place */
  cairo_translate(cr, dx, dy);
  _draw_clb(ctx, site);
  cairo_translate(cr, -dx, -dy);
}

void
draw_clb_new(drawing_context_t *ctx,
	 unsigned x, unsigned y,
	 csite_descr_t *site) {
  cairo_t *cr = ctx->cr;
  /* can be computed incrementally with only one addition */
  double dx = x * SITE_WIDTH, dy = y * SITE_HEIGHT;

  /* don't draw, do a compositing operation */
  cairo_save (cr);
  cairo_translate(cr, dx, dy);
  _draw_clb_new(ctx, site);
  cairo_restore (cr);
  //cairo_translate(cr, -dx, -dy);
}

/* Drawing of regular LUT */
site_draw_t draw_table[NR_SITE_TYPE] = {
  [CLB] = draw_clb_new,
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
void
draw_chip(drawing_context_t *ctx, chip_descr_t *chip) {
  cairo_t *cr = ctx->cr;
  g_print("Start of draw chip\n");
  cairo_rectangle(cr, 0., 0.,
		  chip->width * SITE_WIDTH,
		  chip->height * SITE_HEIGHT);
  cairo_clip (cr);

  cairo_set_source_rgb (cr, 0., 0., 0.);
  /* paint the clip region */
  cairo_paint (cr);

  /* draw everything in white. This could be per-site or per-site-type */
  cairo_set_source_rgb (cr, 1.0, 1.0, 1.0);
  cairo_set_line_width (cr, 1);

  cairo_select_font_face(cr, NAME_FONT_TYPE,
			 CAIRO_FONT_SLANT_NORMAL,
			 CAIRO_FONT_WEIGHT_NORMAL);
  cairo_set_font_size(cr, NAME_FONT_SIZE);

  /* initialize patterns -- once and for all ? */
  if (!ctx->site_patterns[CLB])
    ctx->site_patterns[CLB] = draw_clb_pattern(ctx);

/*   if (!site_patterns[CLB]) { */
/*     g_print("drawing the group"); */
/*     cairo_push_group_with_content (cr, CAIRO_CONTENT_COLOR); */
/*     iterate_over_sites(chip, draw_site, ctx); */
/*     site_patterns[CLB] = cairo_pop_group (cr); */
/*   } */

/*   cairo_set_source (cr, site_patterns[CLB]); */
/*   g_print("painting"); */
/*   cairo_paint (cr); */
/*   g_print("painting ended"); */

  iterate_over_sites(chip, draw_site, ctx);
  cairo_surface_flush ( cairo_get_target(cr) );
  g_print("End of draw chip\n");
}

static void
diff_time(struct timeval *start, struct timeval *end) {
  long usec, sec;
  /* returns the time difference in microseconds */
  /*   diff = (unsigned long)(difftime(end->tv_sec,start->tv_sec) *
       1,000,000,000); */
  usec = end->tv_usec - start->tv_usec;
  sec = end->tv_sec - start->tv_sec;
  if (usec < 0) {
    sec--;
    usec+=1000000;
  }
  g_print("%li seconds and %li microseconds\n",sec, usec);
}

void
draw_chip_monitored(drawing_context_t *ctx, chip_descr_t *chip) {
  struct timeval start, end;

  gettimeofday(&start,NULL);
  draw_chip(ctx, chip);
  gettimeofday(&end, NULL);
  diff_time(&start, &end);
}
/* exported functions do a bunch of initialization */

/* First, initialize the structure to default value. cairo_t is passed
   from above as we are surface-agnostic in this file (rendering to pdf
   or to screen)
*/

static inline void
init_drawing_context(drawing_context_t *ctx) {
  unsigned i;
  ctx->cr = NULL;
  ctx->text = TRUE;
  for (i = 0; i < NR_SITE_TYPE; i++) {
    ctx->site_patterns[i] = NULL;
    ctx->site_line_patterns[i] = NULL;
  }
}

drawing_context_t *
drawing_context_create() {
  drawing_context_t *ctx = g_new(drawing_context_t, 1);
  init_drawing_context(ctx);
  return ctx;
}

static inline void
safe_cairo_pattern_destroy(cairo_pattern_t *pat) {
  if (pat)
    cairo_pattern_destroy(pat);
}

void
drawing_context_destroy(drawing_context_t *ctx) {
  unsigned i;
  /* cleanup the patterns */
  for (i = 0; i < NR_SITE_TYPE; i++) {
    safe_cairo_pattern_destroy(ctx->site_patterns[i]);
    safe_cairo_pattern_destroy(ctx->site_line_patterns[i]);
  }
  g_free(ctx);
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
  init_drawing_context(&ctx);
  set_cairo_context(&ctx, cr);

  draw_chip(&ctx, chip);

  cairo_surface_flush(sr);
  cairo_show_page(cr);

  cairo_destroy(cr);
}

