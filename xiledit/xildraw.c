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
#include "debitlog.h"

/* for timing analysis */
#include <glib.h>

G_DEFINE_TYPE (EggXildrawFace, egg_xildraw_face, GTK_TYPE_DRAWING_AREA);

//static gboolean egg_xildraw_face_realize (GtkWidget *widget, GdkEventConfigure *event);
static gboolean egg_xildraw_face_configure (GtkWidget *widget, GdkEventConfigure *event);
static gboolean egg_xildraw_face_expose (GtkWidget *xildraw, GdkEventExpose *event);

static void egg_xildraw_face_finalize (EggXildrawFace *self);
static gboolean egg_xildraw_key_press_event(GtkWidget *widget,
					    GdkEventKey *event);
static void
egg_xildraw_size_request (GtkWidget      *widget,
			  GtkRequisition *requisition);

static void
egg_xildraw_face_class_init (EggXildrawFaceClass *class)
{
  GtkWidgetClass *widget_class;

  widget_class = GTK_WIDGET_CLASS (class);

  widget_class->configure_event = egg_xildraw_face_configure;
  widget_class->expose_event = egg_xildraw_face_expose;
  widget_class->size_request = egg_xildraw_size_request;
  widget_class->key_press_event = egg_xildraw_key_press_event;

  /* bind finalizing function */
  G_OBJECT_CLASS (class)->finalize =
    (GObjectFinalizeFunc) egg_xildraw_face_finalize;
}

static void egg_xildraw_face_finalize (EggXildrawFace *self)
{
  drawing_context_t *ctx = self->ctx;
  GdkPixmap *pixmap = self->pixmap;

  if (pixmap) {
    self->pixmap = NULL;
    g_object_unref(pixmap);
  }

  if (ctx) {
    self->ctx = NULL;
    drawing_context_destroy (ctx);
  }

}

static void
egg_xildraw_face_init (EggXildrawFace *xildraw)
{
  /* should we do the whole bunch of initialization here ?
     This would be cool, as we would only need code xilinx code here,
     not in the editor itself */
  xildraw->ctx = NULL;
  xildraw->nlz = NULL;
  xildraw->pixmap = NULL;

  /* gtk initialization */
  GTK_WIDGET_SET_FLAGS(GTK_WIDGET (xildraw), GTK_CAN_FOCUS);
  /*   gtk_widget_set_flags(GTK_WIDGET (xildraw), GTK_CAN_FOCUS); */
  /*   drawing_area.grab_focus() */
  gtk_widget_add_events (GTK_WIDGET (xildraw), GDK_KEY_PRESS_MASK);

  debit_log(L_GUI, "xildraw init");
}

static void
diff_time(GTimeVal *start, GTimeVal *end) {
  glong usec, sec;
  usec = end->tv_usec - start->tv_usec;
  sec = end->tv_sec - start->tv_sec;
  if (usec < 0) {
    sec -= 1;
    usec += G_USEC_PER_SEC;
  }
  g_print("%li seconds and %li microseconds\n",sec, usec);
}

static void
draw (EggXildrawFace *draw, cairo_t *cr)
{
  drawing_context_t *ctx = draw->ctx;
  bitstream_analyzed_t *nlz = draw->nlz;

  GTimeVal start, end;
  g_get_current_time(&start);

  draw_chip(ctx, nlz->chip);
  draw_all_wires(ctx, nlz);

  g_get_current_time(&end);
  diff_time(&start, &end);

}

/* This function redraws the back buffer */
static void
egg_xildraw_pixmap_recompute (EggXildrawFace *xildraw) {
  GdkPixmap *pixmap = xildraw->pixmap;
  drawing_context_t *ctx = xildraw->ctx;
  bitstream_analyzed_t *nlz = xildraw->nlz;
  cairo_t *cr;

  const double zoom = ctx->zoom;
  const chip_descr_t *chip = xildraw->nlz->chip;
  const unsigned dimx = chip_drawing_width(chip) * zoom;
  const unsigned dimy = chip_drawing_height(chip) * zoom;

  /* Get a cairo_t */
  cr = gdk_cairo_create (pixmap);
  set_cairo_context(ctx, cr);

  /* generate the patterns before clipping */
  generate_patterns(ctx, nlz->chip);

  cairo_rectangle (cr, 0, 0, dimx, dimy);
  cairo_clip (cr);

  draw (xildraw, cr);

  destroy_patterns(ctx);
  cairo_destroy (cr);
}

static void
egg_xildraw_pixmap_realloc (EggXildrawFace *xildraw) {
  GtkWidget *widget = GTK_WIDGET (xildraw);
  GdkPixmap *pixmap = xildraw->pixmap;
  const drawing_context_t *ctx = xildraw->ctx;
  const double zoom = ctx->zoom;
  const chip_descr_t *chip = xildraw->nlz->chip;
  const unsigned dimx = chip_drawing_width(chip) * zoom;
  const unsigned dimy = chip_drawing_height(chip) * zoom;

  if (pixmap)
    g_object_unref(pixmap);

  debit_log(L_GUI, "pixmap %i x %i", dimx, dimy);
  pixmap = gdk_pixmap_new(widget->window, dimx, dimy, -1);

  if (!pixmap)
    g_warning("HEEEEEEEEEEEEE");

  xildraw->pixmap = pixmap;

  egg_xildraw_pixmap_recompute(xildraw);
}

static gboolean
egg_xildraw_face_configure (GtkWidget *widget, GdkEventConfigure *event)
{
  EggXildrawFace *xildraw = EGG_XILDRAW_FACE(widget);
  GdkPixmap *pixmap = xildraw->pixmap;

  debit_log(L_GUI, "configure event");

  if (!pixmap)
    egg_xildraw_pixmap_realloc(xildraw);

  /* fall through to expose */
  return TRUE;
}

static gboolean
egg_xildraw_face_expose (GtkWidget *widget, GdkEventExpose *event)
{
  EggXildrawFace *xildraw = EGG_XILDRAW_FACE(widget);
  GdkPixmap *pixmap = xildraw->pixmap;
  double zoom = gtk_adjustment_get_value(xildraw->zoomadjust);
  unsigned
    x_offset = gtk_adjustment_get_value(xildraw->hadjust) * zoom,
    y_offset = gtk_adjustment_get_value(xildraw->vadjust) * zoom;

  debit_log(L_GUI, "expose event");

  /* Just redisplay the bitmap correctly */
  gdk_draw_drawable(widget->window,
		    widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
		    pixmap,
		    event->area.x + x_offset, event->area.y + y_offset,
		    event->area.x, event->area.y,
		    event->area.width, event->area.height);

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
  EggXildrawFace *xildraw = g_object_new (EGG_TYPE_XILDRAW_FACE, NULL);
  drawing_context_t *ctx;

  debit_log(L_GUI, "new called");

  xildraw->nlz = nlz;
  ctx = drawing_context_create();
  xildraw->ctx = ctx;

  /* These for now are an absolute position in the view -- in cairo
     coordinates */
  {
    GtkAdjustment *zoomadjust = GTK_ADJUSTMENT(gtk_adjustment_new (0.1, 0.01, 10.0,
								   0.1, 0.3, 0.0));
    xildraw->zoomadjust = zoomadjust;
    ctx->zoom = gtk_adjustment_get_value(zoomadjust);
  }

  {
      const chip_descr_t *chip = nlz->chip;
      const unsigned dimx = chip_drawing_width(chip);
      const unsigned dimy = chip_drawing_height(chip);

      xildraw->vadjust = GTK_ADJUSTMENT(gtk_adjustment_new (0.0, 0.0, dimy,
							    50, 200, 0.0));

      xildraw->hadjust = GTK_ADJUSTMENT(gtk_adjustment_new (0.0, 0.0, dimx,
							    50, 200, 0.0));
  }

  return GTK_WIDGET(xildraw);
}

static void
xildraw_move(EggXildrawFace *xildraw, int dx, int dy) {
  GtkAdjustment *xadjust = xildraw->hadjust;
  GtkAdjustment *yadjust = xildraw->vadjust;
  double x, y;

  x = gtk_adjustment_get_value(xadjust);
  x += dx;
  gtk_adjustment_set_value(xadjust, x);

  y = gtk_adjustment_get_value(yadjust);
  y += dy;
  gtk_adjustment_set_value(yadjust, y);

  /* Should be in callback for set */
  egg_xildraw_redraw (xildraw);
  return;
}

static void
egg_xildraw_size_request (GtkWidget      *widget,
			  GtkRequisition *requisition)
{
  EggXildrawFace *xildraw = EGG_XILDRAW_FACE(widget);
  GtkAdjustment *xadjust = xildraw->hadjust;
  GtkAdjustment *yadjust = xildraw->vadjust;
  double width, height;
  double zoom = gtk_adjustment_get_value(xildraw->zoomadjust);

  g_object_get (G_OBJECT(xadjust), "upper", &width, NULL);
  g_object_get (G_OBJECT(yadjust), "upper", &height, NULL);

  requisition->width = zoom * width;
  requisition->height = zoom * height;
}

static void
xildraw_zoom(EggXildrawFace *xildraw,
	     gdouble zoom_level) {
  drawing_context_t *ctx = xildraw->ctx;
  GtkAdjustment *zoomadjust = xildraw->zoomadjust;
  double zoom;

  zoom = gtk_adjustment_get_value(zoomadjust);
  gtk_adjustment_set_value(zoomadjust, zoom * zoom_level);

  /* Should be done in callback from zoom -- thus the adjustment could
     be set outside of the widget */
  ctx->zoom = gtk_adjustment_get_value(zoomadjust);
  egg_xildraw_pixmap_realloc (xildraw);
  egg_xildraw_redraw (xildraw);
}

#include <gdk/gdkkeysyms.h>

static gboolean
egg_xildraw_key_press_event(GtkWidget *widget,
			    GdkEventKey *event)
{
  EggXildrawFace *xildraw = EGG_XILDRAW_FACE(widget);
  gint k, step = 150;
  gint x_move = 0, y_move = 0;
  //  step = 100 / net->zooms[net->level];

  debit_log(L_GUI, "key press event");

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
  case GDK_plus:
  case GDK_KP_Add:
    xildraw_zoom(xildraw, 0.75);
    break;
  case GDK_minus:
  case GDK_KP_Subtract:
    xildraw_zoom(xildraw, 1.0/0.75);
    break;
  }

  if (x_move || y_move)
    xildraw_move(xildraw, x_move * step, y_move * step);

  return FALSE;
}
