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

/**
 * xildraw.h
 *
 * A GTK+ widget that implements a xilinx bitstream display
 *
 */

#ifndef __EGG_XILDRAW_FACE_H__
#define __EGG_XILDRAW_FACE_H__

#include <gtk/gtk.h>

#include "analysis.h"
#include "bitdraw.h"

G_BEGIN_DECLS

#define EGG_TYPE_XILDRAW_FACE          (egg_xildraw_face_get_type ())
#define EGG_XILDRAW_FACE(obj)          (G_TYPE_CHECK_INSTANCE_CAST ((obj), EGG_TYPE_XILDRAW_FACE, EggXildrawFace))
#define EGG_XILDRAW_FACE_CLASS(obj)    (G_TYPE_CHECK_CLASS_CAST ((obj), EGG_XILDRAW_FACE, EggXildrawFaceClass))
#define EGG_IS_XILDRAW_FACE(obj)       (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EGG_TYPE_XILDRAW_FACE))
#define EGG_IS_XILDRAW_FACE_CLASS(obj) (G_TYPE_CHECK_CLASS_TYPE ((obj), EFF_TYPE_XILDRAW_FACE))
#define EGG_XILDRAW_FACE_GET_CLASS     (G_TYPE_INSTANCE_GET_CLASS ((obj), EGG_TYPE_XILDRAW_FACE, EggXildrawFaceClass))

typedef struct _EggXildrawFace      EggXildrawFace;
typedef struct _EggXildrawFaceClass EggXildrawFaceClass;

struct _EggXildrawFace
{
  GtkDrawingArea parent;

  /* offscreen buffer state */
  GdkPixmap *pixmap;
  drawing_context_t *ctx;
  bitstream_analyzed_t *nlz;

  /* Adjustments for position and zoom */
  GtkAdjustment *vadjust;
  GtkAdjustment *hadjust;
  GtkAdjustment *zoomadjust;

  /* state variables for dragging */
  gboolean dragging;
  double drag_anchor_x;
  double drag_anchor_y;
  double drag_ofs_start_x;
  double drag_ofs_start_y;

  /* context menu */
  GtkMenu *menu;
};

struct _EggXildrawFaceClass
{
  GtkDrawingAreaClass parent_class;
};

GtkType egg_xildraw_face_get_type (void);
GtkWidget *egg_xildraw_face_new (bitstream_analyzed_t *);
void egg_xildraw_adapt_window(EggXildrawFace *xildraw, GtkWindow *window);

/* Quirk ! */
void egg_xildraw_fullscreen(EggXildrawFace *self);
void egg_xildraw_unfullscreen(EggXildrawFace *self);
void egg_xildraw_site_names(EggXildrawFace *self, gboolean val);
void egg_xildraw_zoom_fit(EggXildrawFace *self);
void egg_xildraw_zoom_in(EggXildrawFace *xildraw);
void egg_xildraw_zoom_out(EggXildrawFace *xildraw);

G_END_DECLS

#endif /* __EGG_XILDRAW_FACE_H__ */
