/*
 * (C) Copyright 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
 * All rights reserved.
 */

#ifndef _SITES_H
#define _SITES_H

#include <glib.h>

/* This is the coordinates of the site in its
   local typed grid */

typedef struct _coord {
  gint16 x;
  gint16 y;
} site_t;

typedef enum _site_type {
  CLB = 0,
  TTERM, LTERM, BTERM, RTERM,
  TLTERM, LTTERM, LBTERM, BLTERM, BRTERM, RBTERM, RTTERM, TRTERM,
  TTERMBRAM, BTERMBRAM,
  TIOI, LIOI, BIOI, RIOI,
  TIOIBRAM, BIOIBRAM,
  BRAM, BM,
  TL, BL, BR, TR,
  M, CLKT, CLKB, GCLKC, GCLKH, GCLKHBRAM,
  SITE_TYPE_NEUTRAL,
  NR_SITE_TYPE,
} __attribute__((packed)) site_type_t;

/* more compact vision of the data */
typedef struct _site_descr {
  site_type_t type;
  site_t type_coord;
} csite_descr_t;

/* This type refers to a site pointer inside the database, which may be
   used for pointer arithmetic to get back to the site's global
   coordinates. Actually I should model this on the wiredb model and
   just have a uint16t here. Or choose something. */
typedef csite_descr_t *site_ref_t;

typedef struct _chip_descr {
  unsigned width;
  unsigned height;
  csite_descr_t *data;
} chip_descr_t;

/* get a site by its global coordinates */
static inline site_ref_t
get_global_site(const chip_descr_t *chip,
		unsigned x, unsigned y) {
  unsigned width = chip->width;
  g_assert(x < width);
  g_assert(y < chip->height);
  return &chip->data[y * width + x];
}

static inline site_ref_t
translate_global_site(const chip_descr_t *chip,
		      site_ref_t site, int dx, int dy) {
  unsigned width = chip->width;
  unsigned offset = site - chip->data;
  unsigned x = offset % width;
  unsigned y = offset / width;
  /* check that we stay within bound */
  return get_global_site(chip, x+dx, y+dy);
}

/* The returned string is allocated and should be freed */
void sprint_csite(gchar *data, const csite_descr_t *site);

typedef void (*site_iterator_t)(unsigned site_x, unsigned site_y,
				site_ref_t site, gpointer dat);

void iterate_over_sites(const chip_descr_t *chip,
			site_iterator_t fun, gpointer data);
void iterate_over_typed_sites(const chip_descr_t *chip,
			      site_iterator_t fun, gpointer data);

void release_chip(chip_descr_t *chip);
chip_descr_t *get_chip(const gchar *datadir,
		       const gchar *chipname);

void print_chip(chip_descr_t *chip);

#endif /* _SITES_H */

