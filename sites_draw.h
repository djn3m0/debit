/*
 * (C) Copyright 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
 * All rights reserved.
 */

#include <cairo.h>
#include <glib.h>
#include "sites.h"
#include "wiring.h"

typedef struct _drawing_context {
  cairo_t *cr;
  gboolean text;

  /* structural information */
  chip_descr_t *chip;
  wire_db_t *wiredb;
} drawing_context_t;

cairo_t *
destroy_drawing_context(drawing_context_t *ctx);

drawing_context_t *
create_drawing_context(const cairo_t *cr);
