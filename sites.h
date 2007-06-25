/*
 * (C) Copyright 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
 * All rights reserved.
 */

#ifndef _SITES_H
#define _SITES_H

#include <glib.h>
#include <stdlib.h>
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

typedef uint8_t  slice_index_t;
#define BITAT(x,off) ((x >> off) & 1)

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

static inline site_type_t
site_type(const chip_descr_t *chip,
	  const site_ref_t site) {
  return get_site(chip, site)->type;
}

static inline unsigned
clip_val(const int delta, const unsigned clip) {
  return delta < 0 ? 0 : ((unsigned)delta > clip ? clip : (unsigned)delta);
}

static inline site_ref_t
translate_global_site(const chip_descr_t *chip,
		      const site_ref_t site,
		      const int dx, const int dy) {
  const unsigned width = chip->width, height = chip->height;
  const unsigned offset = site_index(site);
  const unsigned x = offset % width;
  const unsigned y = offset / width;
  int newx = x+dx, newy = y+dy;
  /* check that we stay within bound */
  if ((unsigned)(newx-1) < width - 2 &&
      (unsigned)(newy-1) < height - 2) {
    return newx + width * newy;
  }
  return SITE_NULL;
}

static inline site_ref_t
project_global_site(const chip_descr_t *chip,
		    const site_ref_t site,
		    const int dx, const int dy,
		    unsigned *offxy) {
  const unsigned width = chip->width, height = chip->height;
  const unsigned offset = site_index(site);
  const unsigned x = offset % width;
  const unsigned y = offset / width;
  int newx = x+dx, newy = y+dy;
  /* We quantify the overflow and return the site projection */
  unsigned offx = ((unsigned)(newx-1) < width - 2) ? 0 : newx <= 1 ? (unsigned)(1-newx) : newx - width + 2;
  unsigned offy = ((unsigned)(newy-1) < height - 2) ? 0 : newy <= 1 ? (unsigned)(1-newy) : newy - height + 2;

  *offxy = offx + offy;
  //  g_warning("Returning offset %i", *offxy);
/*   if ((offx + offy) == 0) */
/*     return (1+clip_val(newx-1, width-2)) + width * (1+clip_val(newy-1, height-2)); */
/*   else */
  return clip_val(newx, width-1) + width * clip_val(newy, height-1);
}

#define MAX_SITE_NLEN 20
int snprint_csite(gchar *buf, const size_t bufs,
		  const csite_descr_t *site,
		  const unsigned gx, const unsigned gy);
int snprint_switch(gchar *data, const size_t bufs,
		   const chip_descr_t *chip,
		   const site_ref_t swb);
void snprint_slice(gchar *buf, size_t buf_len, const chip_descr_t *chip,
		   const csite_descr_t *site, const slice_index_t slice);

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

