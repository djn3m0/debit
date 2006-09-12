/*
 * (C) Copyright 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
 * All rights reserved.
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

#include "sites.h"
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
  /* < public > */
  drawing_context_t *ctx;
  chip_descr_t *chip;
};

struct _EggXildrawFaceClass
{
  GtkDrawingAreaClass parent_class;
};

GtkWidget *egg_xildraw_face_new (void);

G_END_DECLS

#endif
