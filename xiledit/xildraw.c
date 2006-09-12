/**
 * xildraw.c
 *
 * A GTK+ widget that implements a xilinx bitstream display
 *
 */

#include <gtk/gtk.h>
#include <cairo.h>
#include "xildraw.h"

#include "sites.h"
#include "bitdraw.h"

G_DEFINE_TYPE (EggXildrawFace, egg_xildraw_face, GTK_TYPE_DRAWING_AREA);

static gboolean egg_xildraw_face_expose (GtkWidget *xildraw, GdkEventExpose *event);

static void egg_xildraw_face_finalize (EggXildrawFace *self);

static void
egg_xildraw_face_class_init (EggXildrawFaceClass *class)
{
  GtkWidgetClass *widget_class;

  widget_class = GTK_WIDGET_CLASS (class);

  widget_class->expose_event = egg_xildraw_face_expose;

  /* bind finalizing function */
  G_OBJECT_CLASS (class)->finalize =
    (GObjectFinalizeFunc) egg_xildraw_face_finalize;
}

static void egg_xildraw_face_finalize (EggXildrawFace *self)
{
  drawing_context_t *ctx = self->ctx;
  chip_descr_t *chip = self->chip;

  /* dealloc all needed */
  if (ctx) {
    self->ctx = NULL;
    drawing_context_create (ctx);
  }

  if (chip) {
    self->chip = NULL;
    release_chip (chip);
  }

  g_print("finalized\n");
}

static void
egg_xildraw_face_init (EggXildrawFace *xildraw)
{
  /* should we do the whole bunch of initialization here ?
     This would be cool, as we would only need code xilinx code here,
     not in the editor itself */
  xildraw->ctx = NULL;
  xildraw->chip = NULL;

  g_print("init\n");
}

static void
draw (EggXildrawFace *draw, cairo_t *cr)
{
/* 	double x, y; */
/* 	double radius; */
/* 	int i; */

/* 	x = xildraw->allocation.x + xildraw->allocation.width / 2; */
/* 	y = xildraw->allocation.y + xildraw->allocation.height / 2; */
/* 	radius = MIN (xildraw->allocation.width / 2, */
/* 		      xildraw->allocation.height / 2) - 5; */

/* 	/\* xildraw back *\/ */
/* 	cairo_arc (cr, x, y, radius, 0, 2 * M_PI); */
/* 	cairo_set_source_rgb (cr, 1, 1, 1); */
/* 	cairo_fill_preserve (cr); */
/* 	cairo_set_source_rgb (cr, 0, 0, 0); */
/* 	cairo_stroke (cr); */

/* 	/\* xildraw ticks *\/ */
/* 	for (i = 0; i < 12; i++) */
/* 	{ */
/* 		int inset; */

/* 		cairo_save (cr); /\* stack-pen-size *\/ */

/* 		if (i % 3 == 0) */
/* 		{ */
/* 			inset = 0.2 * radius; */
/* 		} */
/* 		else */
/* 		{ */
/* 			inset = 0.1 * radius; */
/* 			cairo_set_line_width (cr, 0.5 * */
/* 					      cairo_get_line_width (cr)); */
/* 		} */

/* 		cairo_move_to (cr, */
/* 			       x + (radius - inset) * cos (i * M_PI / 6), */
/* 			       y + (radius - inset) * sin (i * M_PI / 6)); */
/* 		cairo_line_to (cr, */
/* 			       x + radius * cos (i * M_PI / 6), */
/* 			       y + radius * sin (i * M_PI / 6)); */
/* 		cairo_stroke (cr); */
/* 		cairo_restore (cr); /\* stack-pen-size *\/ */
/* 	} */
  drawing_context_t *ctx = draw->ctx;
  chip_descr_t *chip = draw->chip;
  g_print("draw\n");
  draw_chip(ctx, chip);
}

static gboolean
egg_xildraw_face_expose (GtkWidget *_xildraw, GdkEventExpose *event)
{
  cairo_t *cr;
  EggXildrawFace *xildraw = EGG_XILDRAW_FACE(_xildraw);

  /* get a cairo_t */
  cr = gdk_cairo_create (_xildraw->window);

  cairo_rectangle (cr,
		   event->area.x, event->area.y,
		   event->area.width, event->area.height);
  cairo_clip (cr);

  set_cairo_context(xildraw->ctx, cr);
  draw (xildraw, cr);

  cairo_destroy (cr);

  return FALSE;
}

GtkWidget *
egg_xildraw_face_new (void)
{
  EggXildrawFace *ret = g_object_new (EGG_TYPE_XILDRAW_FACE, NULL);
  /* do a bunch of things */
  ret->chip = get_chip("/home/jb/chip/","xc2v2000");
  ret->ctx = drawing_context_create();
  return GTK_WIDGET(ret);
}
