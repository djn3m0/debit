/**
 * xildraw.c
 *
 * A GTK+ widget that implements a xilinx bitstream display
 *
 */

#include <gtk/gtk.h>
#include <cairo.h>
#include "xildraw.h"

#include "analysis.h"
#include "bitdraw.h"

G_DEFINE_TYPE (EggXildrawFace, egg_xildraw_face, GTK_TYPE_DRAWING_AREA);

static gboolean egg_xildraw_face_expose (GtkWidget *xildraw, GdkEventExpose *event);

static void egg_xildraw_face_finalize (EggXildrawFace *self);
static gboolean egg_xildraw_key_press_event(GtkWidget *widget,
					    GdkEventKey *event);
static void
egg_xildraw_face_class_init (EggXildrawFaceClass *class)
{
  GtkWidgetClass *widget_class;

  widget_class = GTK_WIDGET_CLASS (class);

  widget_class->expose_event = egg_xildraw_face_expose;
  widget_class->key_press_event = egg_xildraw_key_press_event;

  /* bind finalizing function */
  G_OBJECT_CLASS (class)->finalize =
    (GObjectFinalizeFunc) egg_xildraw_face_finalize;
}

static void egg_xildraw_face_finalize (EggXildrawFace *self)
{
  drawing_context_t *ctx = self->ctx;
  bitstream_analyzed_t *nlz = self->nlz;

  /* dealloc all needed */
  if (ctx) {
    self->ctx = NULL;
    drawing_context_create (ctx);
  }

  if (nlz) {
    self->nlz = NULL;
    free_analysis(nlz);
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
  xildraw->nlz = NULL;

  /* gtk initialization */
  GTK_WIDGET_SET_FLAGS(GTK_WIDGET (xildraw), GTK_CAN_FOCUS);
  /*   gtk_widget_set_flags(GTK_WIDGET (xildraw), GTK_CAN_FOCUS); */
  /*   drawing_area.grab_focus() */
  gtk_widget_add_events (GTK_WIDGET (xildraw), GDK_KEY_PRESS_MASK);

  g_print("init\n");
}

static void
draw (EggXildrawFace *draw, cairo_t *cr)
{
  drawing_context_t *ctx = draw->ctx;
  bitstream_analyzed_t *nlz = draw->nlz;
  g_print("draw\n");
  draw_chip_monitored(ctx, nlz->chip);
}

static gboolean
egg_xildraw_face_expose (GtkWidget *widget, GdkEventExpose *event)
{
  cairo_t *cr;
  EggXildrawFace *xildraw = EGG_XILDRAW_FACE(widget);
  (void) event;

  g_print("expose event");

  /* get a cairo_t */
  cr = gdk_cairo_create (widget->window);

  cairo_rectangle (cr,
		   event->area.x, event->area.y,
		   event->area.width, event->area.height);
  cairo_clip (cr);

  set_cairo_context(xildraw->ctx, cr);
  draw (xildraw, cr);

  cairo_destroy (cr);

  return FALSE;
}

static void
egg_xildraw_redraw (EggXildrawFace *xildraw)
{
  GtkWidget *widget;
  GdkRegion *region;

  widget = GTK_WIDGET (xildraw);

  if (!widget->window)
    return;

  region = gdk_drawable_get_clip_region (widget->window);
  /* redraw the cairo canvas completely by exposing it */
  gdk_window_invalidate_region (widget->window, region, TRUE);
  gdk_window_process_updates (widget->window, TRUE);

  gdk_region_destroy (region);
}

GtkWidget *
egg_xildraw_face_new (bitstream_analyzed_t *nlz)
{
  EggXildrawFace *ret = g_object_new (EGG_TYPE_XILDRAW_FACE, NULL);
  /* do a bunch of things */
  ret->nlz = nlz;
  ret->ctx = drawing_context_create();
  return GTK_WIDGET(ret);
}

static void
xildraw_move(EggXildrawFace *xildraw,
	     unsigned dx, unsigned dy) {
  drawing_context_t *ctx = xildraw->ctx;
  ctx->x_offset += dx;
  ctx->y_offset += dy;
  egg_xildraw_redraw (xildraw);
  return;
}

static void
xildraw_zoom(EggXildrawFace *xildraw,
	     gdouble zoom_level) {
  drawing_context_t *ctx = xildraw->ctx;
  ctx->zoom *= zoom_level;
  egg_xildraw_redraw (xildraw);
}

#include <gdk/gdkkeysyms.h>

static gboolean
egg_xildraw_key_press_event(GtkWidget *widget,
			    GdkEventKey *event)
{
  EggXildrawFace *xildraw = EGG_XILDRAW_FACE(widget);
  gint k, step = 100;
  gint x_move = 0, y_move = 0;
  //  step = 100 / net->zooms[net->level];

  g_print("key press event !");

  k = event->keyval;

  switch(k) {
  case GDK_Left:
    x_move = -1;
    break;
  case GDK_Right:
    x_move = 1;
    break;
  case GDK_Up:
    y_move = -1;
    break;
  case GDK_Down:
    y_move = 1;
    break;
/*   case GDK_Page_Up: */
/*     netdraw_move(gui, 0,-4*step); */
/*     break; */
/*   case GDK_Page_Down: */
/*     netdraw_move(gui, 0,+4*step); */
/*     break; */
/*   case GDK_plus: */
  case GDK_KP_Add:
  case GDK_plus:
    xildraw_zoom(xildraw, 0.75);
    break;
  case GDK_minus:
  case GDK_KP_Subtract:
    xildraw_zoom(xildraw, 1/0.75);
    break;
  }

  if (x_move || y_move)
    xildraw_move(xildraw, x_move * step, y_move * step);

  return FALSE;
}
