/**
 * xildraw.c
 *
 * A GTK+ widget that implements a xilinx bitstream display
 *
 */

#include <gtk/gtk.h>
#include <cairo.h>
#include "xildraw.h"
#include "interface.h"

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
static gboolean egg_xildraw_button_press_event(GtkWidget *widget,
					       GdkEventButton *event);
static gboolean egg_xildraw_motion_notify_event(GtkWidget *widget,
						GdkEventMotion *event);
static gboolean egg_xildraw_scroll_event(GtkWidget *widget, GdkEventScroll *event);

static void
egg_xildraw_face_class_init (EggXildrawFaceClass *class)
{
  GtkWidgetClass *widget_class;

  widget_class = GTK_WIDGET_CLASS (class);

  widget_class->configure_event = egg_xildraw_face_configure;
  widget_class->expose_event = egg_xildraw_face_expose;
  widget_class->key_press_event = egg_xildraw_key_press_event;
  widget_class->button_press_event = egg_xildraw_button_press_event;
  widget_class->motion_notify_event = egg_xildraw_motion_notify_event;
  widget_class->scroll_event = egg_xildraw_scroll_event;

  /* bind finalizing function */
  G_OBJECT_CLASS (class)->finalize =
    (GObjectFinalizeFunc) egg_xildraw_face_finalize;
}

static void egg_xildraw_face_finalize (EggXildrawFace *self)
{
  drawing_context_t *ctx = self->ctx;
  GdkPixmap *pixmap = self->pixmap;
  GtkAdjustment
    *zoomadjust = self->zoomadjust,
    *vadjust = self->vadjust,
    *hadjust = self->hadjust;

  if (pixmap) {
    self->pixmap = NULL;
    g_object_unref(pixmap);
  }

  if (zoomadjust) {
    self->zoomadjust = NULL;
    g_object_unref(zoomadjust);
  }

  if (vadjust) {
    self->vadjust = NULL;
    g_object_unref(vadjust);
  }

  if (hadjust) {
    self->hadjust = NULL;
    g_object_unref(hadjust);
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
  gtk_widget_add_events (GTK_WIDGET (xildraw), GDK_BUTTON_PRESS_MASK);
  gtk_widget_add_events (GTK_WIDGET (xildraw), GDK_BUTTON_RELEASE_MASK);

  /* add / remove this on drag start -- we usually don't want to handle it */
  //  gtk_widget_add_events (GTK_WIDGET (xildraw),
  //  GDK_POINTER_MOTION_MASK);
  gtk_widget_add_events (GTK_WIDGET (xildraw), GDK_BUTTON2_MOTION_MASK);
  gtk_widget_add_events (GTK_WIDGET (xildraw), GDK_SCROLL_MASK);

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

/*
 * Principles of pixmap cache:
 *
 * 1/ The expose event is just a redraw of the backing pixmap to screen
 * 2/ The other events, such as moving, zooming, resizing, realloc the
 * pixmap if needed, recompute the pixmap. Possible optimizations
 * include recomputing only the relevant portion of the pixmap, in case
 * of a move.
 */

static void egg_xildraw_pixmap_recompute (EggXildrawFace *xildraw);

/*
 * Called on surface reconfig. For now, only alloc a pixmap which fits
 * the window size.
 */

static void
egg_xildraw_pixmap_realloc (EggXildrawFace *xildraw) {
  GtkWidget *widget = GTK_WIDGET (xildraw);
  GdkPixmap *pixmap = xildraw->pixmap;

  if (pixmap)
    g_object_unref(pixmap);

  pixmap = gdk_pixmap_new(widget->window,
                          widget->allocation.width,
                          widget->allocation.height, -1);

  xildraw->pixmap = pixmap;
  egg_xildraw_pixmap_recompute(xildraw);
}

/* When does this happen ? */

static gboolean
egg_xildraw_face_configure (GtkWidget *widget, GdkEventConfigure *event)
{
  EggXildrawFace *xildraw = EGG_XILDRAW_FACE(widget);
  debit_log(L_GUI, "configure event");

  egg_xildraw_pixmap_realloc(xildraw);

  /* Set the page size of the controls
     This should only be changed on zoom change */
  {
    double zoom = gtk_adjustment_get_value(xildraw->zoomadjust);
    GtkAdjustment *xadjust = xildraw->hadjust;
    GtkAdjustment *yadjust = xildraw->vadjust;
    double
      width = widget->allocation.width / zoom,
      height = widget->allocation.height / zoom;

    g_object_set (G_OBJECT(xadjust), "page-size", width, NULL);
    g_object_set (G_OBJECT(yadjust), "page-size", height, NULL);
  }

  /* fall through to expose */
  return TRUE;
}

/*
 * Just blit the pixmap to screen
 */

static gboolean
egg_xildraw_face_expose (GtkWidget *widget, GdkEventExpose *event)
{
  EggXildrawFace *xildraw = EGG_XILDRAW_FACE(widget);
  GdkPixmap *pixmap = xildraw->pixmap;
  debit_log(L_GUI, "expose event");
  double x = event->area.x, y = event->area.y;

  /* Just redisplay the bitmap correctly */
  gdk_draw_drawable(widget->window,
		    widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
		    pixmap, x, y, x, y,
		    event->area.width, event->area.height);

  /* If dragging, we can ask for the next motion event */

  return FALSE;
}

/*
 * Pixmap recomputation
 */

static void
print_site_area(site_area_t *area) {
  g_print("[%i - %i] x [%i - %i]\n",
	  area->x, area->x+area->width,
	  area->y, area->y+area->height);
}

static void
compute_sites_from_area(site_area_t *range, double zoom,
			double x, double width,
			double y, double height) {
  /* move to the grid, by dividing by zoom */
  unsigned ux = x / zoom;
  unsigned uy = y / zoom;
  unsigned uwidth = width / zoom;
  unsigned uheight = height / zoom;

  /* FIBWWYW */
#define GET_NUM(x, vx) (x) / (vx)

  /* Yes, we should go further than this */
  range->x = GET_NUM(ux, SITE_WIDTH);
  range->width = GET_NUM(uwidth, SITE_WIDTH);
  range->y = GET_NUM(uy, SITE_HEIGHT);
  range->height = GET_NUM(uheight, SITE_HEIGHT);
}

static void
draw_limited (EggXildrawFace *draw, cairo_t *cr,
	      const site_area_t *sites)
{
  drawing_context_t *ctx = draw->ctx;
  bitstream_analyzed_t *nlz = draw->nlz;

  GTimeVal start, end;
  g_get_current_time(&start);

  /* How to specify the limits */
  draw_chip_limited(ctx, nlz->chip);
  draw_all_wires_limited(ctx, nlz, sites);

  g_get_current_time(&end);
  diff_time(&start, &end);

}

/* This function redraws the back buffer */
static void
egg_xildraw_pixmap_recompute (EggXildrawFace *xildraw)
{
  GtkWidget *widget = GTK_WIDGET (xildraw);

  /* Redraw everything, only what's needed */
  double zoom = gtk_adjustment_get_value(xildraw->zoomadjust);
  double
    x_offset = gtk_adjustment_get_value(xildraw->hadjust) * zoom,
    y_offset = gtk_adjustment_get_value(xildraw->vadjust) * zoom;
  drawing_context_t *ctx = xildraw->ctx;
  bitstream_analyzed_t *nlz = xildraw->nlz;
  cairo_t *cr;
  site_area_t range;
  double width = widget->allocation.width,
    height = widget->allocation.height;

  /* get a cairo_t */
  cr = gdk_cairo_create (xildraw->pixmap);
  set_cairo_context(ctx, cr);

  /* generate the patterns before clipping */
  generate_patterns(ctx, nlz->chip);

  /* redraw on the pixmap */
  cairo_rectangle (cr, 0, 0, width, height);
  cairo_clip (cr);

  cairo_translate (cr, -x_offset, -y_offset);

  /* Restrict drawing to the pixmap region */

  compute_sites_from_area(&range, zoom,
			  x_offset, width,
			  y_offset, height);
  print_site_area(&range);

  draw_limited (xildraw, cr, &range);
  destroy_patterns (ctx);
  cairo_destroy (cr);

}

/* This function is called when a redraw is necessary; it just sends an
   expose event for the whole screen */

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

static void
egg_xildraw_adjustment_value_changed (GtkAdjustment *adjustment,
				      gpointer       data)
{
  EggXildrawFace *xildraw = EGG_XILDRAW_FACE(data);

  g_return_if_fail (adjustment != NULL);
  g_return_if_fail (data != NULL);

  /* We could be much more intelligent here by only recomputing the part
     of the pixmap which appeared */
  egg_xildraw_pixmap_recompute (xildraw);
  egg_xildraw_redraw (xildraw);
}

static void
egg_xildraw_adapt_widget(EggXildrawFace *self) {
  GtkWidget *window = gtk_widget_get_parent( GTK_WIDGET(self) );
  egg_xildraw_adapt_window(self, GTK_WINDOW(window));
}

/* Needed for the new implementation */
static void
egg_xildraw_zoom_value_changed (GtkAdjustment *zoomadjust,
				gpointer       data)
{
  EggXildrawFace *xildraw = EGG_XILDRAW_FACE(data);
  drawing_context_t *ctx = xildraw->ctx;

  ctx->zoom = gtk_adjustment_get_value(zoomadjust);
  /* The pixmap has not changed size:
     recompute the pixmap and expose it */
  egg_xildraw_pixmap_recompute (xildraw);
  egg_xildraw_redraw (xildraw);

  /* erk */
  egg_xildraw_adapt_widget(xildraw);
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

  {
    GtkAdjustment *zoomadjust = GTK_ADJUSTMENT(gtk_adjustment_new (0.1, 0.01, 10.0,
								   0.1, 0.3, 0.0));
    gtk_object_ref ( GTK_OBJECT(zoomadjust) );
    xildraw->zoomadjust = zoomadjust;
    ctx->zoom = gtk_adjustment_get_value(zoomadjust);

    gtk_signal_connect (GTK_OBJECT (zoomadjust), "value_changed",
			(GtkSignalFunc) egg_xildraw_zoom_value_changed,
			(gpointer) xildraw);
  }

  /* These for now are an absolute position in the view -- in cairo
     coordinates */
  {
    GtkAdjustment *hadjust, *vadjust;
    const chip_descr_t *chip = nlz->chip;
    const unsigned dimx = chip_drawing_width(chip);
    const unsigned dimy = chip_drawing_height(chip);

    vadjust = GTK_ADJUSTMENT(gtk_adjustment_new (0.0, 0.0, dimy, 50, 200, dimy / 0.1));
    gtk_object_ref ( GTK_OBJECT(vadjust) );
    xildraw->vadjust = vadjust;

    gtk_signal_connect (GTK_OBJECT (vadjust), "value_changed",
			(GtkSignalFunc) egg_xildraw_adjustment_value_changed,
			(gpointer) xildraw);

    hadjust = GTK_ADJUSTMENT(gtk_adjustment_new (0.0, 0.0, dimx, 50, 200, dimx / 0.1));
    gtk_object_ref ( GTK_OBJECT(hadjust) );
    xildraw->hadjust = hadjust;

    gtk_signal_connect (GTK_OBJECT (hadjust), "value_changed",
			(GtkSignalFunc) egg_xildraw_adjustment_value_changed,
			(gpointer) xildraw);
  }

  {
    GtkMenu *menu = GTK_MENU (create_bitcontext());
    gtk_menu_attach_to_widget (menu, GTK_WIDGET(xildraw), NULL);
    xildraw->menu = menu;
  }

  return GTK_WIDGET(xildraw);
}

static void
xildraw_adjust_delta(GtkAdjustment *adjust, int delta) {
  double val;

  if (delta == 0)
    return;

  val = gtk_adjustment_get_value(adjust);
  val += delta;
  gtk_adjustment_set_value(adjust, val);
}

static void
xildraw_scale(GtkAdjustment *zoomadjust,
	      gdouble zoom_level) {
  double zoom;
  if (zoom_level == 1.0)
    return;

  zoom = gtk_adjustment_get_value(zoomadjust);
  gtk_adjustment_set_value(zoomadjust, zoom * zoom_level);
}

static inline void
zoom_in(EggXildrawFace *xildraw) {
  xildraw_scale(xildraw->zoomadjust, 1.0/0.75);
}

static inline void
zoom_out(EggXildrawFace *xildraw) {
  xildraw_scale(xildraw->zoomadjust, 0.75);
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
  case GDK_minus:
  case GDK_KP_Subtract:
    zoom_out(xildraw);
    break;
  case GDK_plus:
  case GDK_KP_Add:
    zoom_in(xildraw);
    break;
  }

  if (x_move)
    xildraw_adjust_delta(xildraw->hadjust, x_move * step);
  if (y_move)
    xildraw_adjust_delta(xildraw->vadjust, y_move * step);

  return FALSE;
}

static gboolean
egg_xildraw_scroll_event(GtkWidget *widget,
			 GdkEventScroll *event) {
  GdkScrollDirection direction = event->direction;
  switch (direction) {
  case GDK_SCROLL_UP:
    zoom_in( EGG_XILDRAW_FACE(widget) );
    return TRUE;
  case GDK_SCROLL_DOWN:
    zoom_out( EGG_XILDRAW_FACE(widget) );
    return TRUE;
  default:
    g_warning("Unhandled scroll direction");
  }
  return FALSE;
}

static gboolean
egg_xildraw_button_press_event(GtkWidget *widget,
			       GdkEventButton *event)
{
  EggXildrawFace *xildraw = EGG_XILDRAW_FACE(widget);

  guint state, button;
  GdkEventType type;

  state = event->state;
  button = event->button;
  type = event->type;

  switch (button) {
    /* left click should do something complicated with select */
  case 1:
    break;
    /* middle click */
  case 2:
    switch (type) {
    case GDK_BUTTON_PRESS:
      xildraw->drag = TRUE;
      gdk_event_get_coords ((GdkEvent *)event,
			    &xildraw->refx,
			    &xildraw->refy);
      break;
    case GDK_BUTTON_RELEASE:
      xildraw->drag = FALSE;
      break;
    default:
      return FALSE;
    }
    return TRUE;
  /* right click */
  case 3:
    if (type == GDK_BUTTON_PRESS) {
      gtk_menu_popup (GTK_MENU (xildraw->menu), NULL, NULL, NULL, NULL,
		      event->button, event->time);
      return TRUE;
    }
  /* mouse wheel usual config */
  case 4:
    zoom_in(xildraw);
    return TRUE;
  case 5:
    zoom_out(xildraw);
    return TRUE;
  default:
    g_warning("Unknown button %i pressed", button);
  }
  return FALSE;
}

/* Mouse movement */
static gboolean
egg_xildraw_motion_notify_event(GtkWidget *widget,
				GdkEventMotion *event)
{
  EggXildrawFace *xildraw = EGG_XILDRAW_FACE(widget);
  gdouble zoom = gtk_adjustment_get_value(xildraw->zoomadjust);

  if (xildraw->drag) {
    gdouble oldx = xildraw->refx, oldy = xildraw->refy;
    gdouble newx, newy;

    /* Compute the motion vector */
    gdk_event_get_coords ((GdkEvent *)event, &newx, &newy);

    /* Redraw at new position. We must switch to device coordinates. */
    xildraw_adjust_delta(xildraw->vadjust, (oldy - newy) / zoom);
    xildraw_adjust_delta(xildraw->hadjust, (oldx - newx) / zoom);

    /* Reset the motion vector */
    xildraw->refx = newx;
    xildraw->refy = newy;

    return TRUE;
  }

  return FALSE;
}

/* Various exported functions to act on the xildraw context. At some
   point, these would need to be converted to signal handlers,
   maybe. For now this does the job */

void
egg_xildraw_adapt_window(EggXildrawFace *xildraw, GtkWindow *window) {
  GtkAdjustment *xadjust = xildraw->hadjust;
  GtkAdjustment *yadjust = xildraw->vadjust;
  double width, height;
  double zoom = gtk_adjustment_get_value(xildraw->zoomadjust);
  GdkGeometry geom;

  g_object_get (G_OBJECT(xadjust), "upper", &width, NULL);
  g_object_get (G_OBJECT(yadjust), "upper", &height, NULL);

  geom.max_width = zoom * width;
  geom.max_height = zoom * height;
  gtk_window_set_geometry_hints (window, GTK_WIDGET(xildraw),
				 &geom, GDK_HINT_MAX_SIZE);

  gtk_window_set_default_size (window, zoom * width, zoom * height);

}

/* Really, these should not be set here. The context menu should
   certainly be drawn from the toplevel window. */
void
egg_xildraw_fullscreen(EggXildrawFace *self) {
  GtkWidget *window = gtk_widget_get_parent( GTK_WIDGET(self) );
  gtk_window_fullscreen ( GTK_WINDOW(window) );
}

void
egg_xildraw_unfullscreen(EggXildrawFace *self) {
  GtkWidget *window = gtk_widget_get_parent( GTK_WIDGET(self) );
  gtk_window_unfullscreen ( GTK_WINDOW(window) );
}

void
egg_xildraw_zoom_fit(EggXildrawFace *self) {
  double new_zoom;
  /* The zoom is so that the pagesize matches the whole buffer width */
  new_zoom = 1.0;
}

void
egg_xildraw_site_names(EggXildrawFace *self, gboolean set) {
  drawing_context_t *ctx = self->ctx;
  ctx->text = set;
  egg_xildraw_redraw (self);
/*   egg_xildraw_pixmap_recompute(self); */
}
