/*
 * (C) Copyright 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
 * All rights reserved.
 */

#ifndef _SITES_H
#define _SITES_H

#include <glib.h>
#include "bitstream_parser.h"

#if defined(VIRTEX2) || defined(SPARTAN3)
#include "sites_v2.h"
#elif defined(VIRTEX4)
#include "sites_v4.h"
#elif defined(VIRTEX5)
#include "sites_v5.h"
#endif

/* This is the coordinates of the site in its
   local typed grid */

typedef struct _coord {
  gint16 x;
  gint16 y;
} site_t;

/* more compact vision of the data */
typedef struct _site_descr {
  site_type_t type;
  site_t type_coord;
} csite_descr_t;

/* This type refers to a site pointer inside the database, which may be
   used for pointer arithmetic to get back to the site's global
   coordinates. Actually I should model this on the wiredb model and
   just have a uint16t here. Or choose something. */
typedef uint16_t site_ref_t;
#define SITE_NULL ((site_ref_t)-1)

typedef struct _chip_descr {
  unsigned width;
  unsigned height;
  csite_descr_t *data;
} chip_descr_t;

/* Describes a rectangular site range */
typedef struct site_area {
  unsigned x;
  unsigned width;
  unsigned y;
  unsigned height;
} site_area_t;

/* get a site index, in-order WRT iterate_over_sites */
static inline unsigned
site_index(const site_ref_t site) {
  return site;
}

/* get a site by its global coordinates */
static inline csite_descr_t *
get_global_site(const chip_descr_t *chip,
		unsigned x, unsigned y) {
  unsigned width = chip->width;
  g_assert(x < width);
  g_assert(y < chip->height);
  return &chip->data[y*width + x];
}

static inline csite_descr_t *
get_site(const chip_descr_t *chip,
	 const site_ref_t ref) {
  return &chip->data[ref];
}

static inline site_ref_t
translate_global_site(const chip_descr_t *chip,
		      site_ref_t site, int dx, int dy) {
  unsigned width = chip->width;
  unsigned offset = site_index(site);
  unsigned x = offset % width;
  unsigned y = offset / width;
  unsigned newx = x+dx, newy = y+dy;
  /* check that we stay within bound */
  if (newx < chip->width && newy < chip->height)
    return newx + width * newy;
  return SITE_NULL;
}

void sprint_csite(gchar *data, const csite_descr_t *site,
		  unsigned gx, unsigned gy);
void sprint_switch(gchar *data, const chip_descr_t *chip,
		   const site_ref_t swb);

typedef void (*site_iterator_t)(unsigned site_x, unsigned site_y,
				csite_descr_t *site, gpointer dat);

void iterate_over_sites(const chip_descr_t *chip,
			site_iterator_t fun, gpointer data);
void iterate_over_typed_sites(const chip_descr_t *chip, site_type_t type,
			      site_iterator_t fun, gpointer data);

void release_chip(chip_descr_t *chip);
chip_descr_t *get_chip(const gchar *datadir, const unsigned chipid);

void print_chip(chip_descr_t *chip);

#endif /* _SITES_H */

