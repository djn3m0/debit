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
  guint16 x;
  guint16 y;
} site_t;

static inline site_t
add_sites(const site_t s1, const site_t s2) {
  site_t added = { .x = s1.x + s2.x, .y = s1.y + s2.y };
  return added;
}

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

typedef uint32_t nsite_ref_t;
#define NSITE_NULL ((nsite_ref_t)-1)

/* Describes a rectangular site range */
typedef struct interval {
  unsigned base;
  unsigned length;
} interval_t;

typedef struct nsite_area {
  /* coordinates of the upper left corner of the area on the subsampled
     grid of places of the same type */
  site_t type_base;
  site_type_t type;
} nsite_area_t;

typedef struct _chip_descr {
  unsigned width;
  unsigned height;
  csite_descr_t *data;
  GData *lookup;
  /* New kind of optimized site database. the descr arrays must
     contain and end-of-line element, with length 0 and base = width+1 */
  unsigned awidth;
  const interval_t *x_descr;
  unsigned aheight;
  const interval_t *y_descr;
  const nsite_area_t *area;
} chip_descr_t;

/* New nsite_ref_t container, on 32 bits. The nsite_ref_t contains 2
   coodinates: the linear coordinates in the nsite_area_t array, and the
   relative coordinate in this space, each on 16 bits. The linear
   coordinate is computed in standard way (y*width+x).
 */
#define AREA_COORD(x) ((x) >> 16)
#define RELATIVE_COORD(x) ((x) & ((1 << 16) - 1))

#define MAKE_AREA(chip, x, y) (x | (y << 8))
#define MAKE_RELATIVE(chip, x, y) (x | (y << 8))
#define GET_RELATIVE_X(chip, lco) (lco & (0xff))
#define GET_RELATIVE_Y(chip, lco) ((lco >> 8) & 0xff)
#define GET_AREA_X(chip, area) (area & (0xff))
#define GET_AREA_Y(chip, area) ((area >> 8) & 0xff)
#define MAKE_NSITE(area, rel) (rel | area << 16)

static inline
int find_index(const interval_t *itv, const unsigned itv_l,
	       const int seek) {
  /* Simple, linear search */
  unsigned coord;
  for (coord = 0; coord < itv_l; coord++) {
    int base = itv[coord].base;
    if (seek < base)
      return (int)(coord-1);
  }
  return -1;
}

/*
 * Translate global coordinates into the nsite system of coordinates.
 * This definitely allows better *everything*
 */

static inline nsite_ref_t
nsite_of_global(const chip_descr_t *chip,
		const unsigned x, const unsigned y) {
  int ycoord, xcoord;
  unsigned x_off, y_off, lco, off;
  xcoord = find_index(chip->x_descr, chip->awidth, x);
  x_off = (xcoord < 0) ? 0 : x - chip->x_descr[xcoord].base;
  ycoord = find_index(chip->y_descr, chip->aheight, y);
  y_off = (ycoord < 0) ? 0 : y - chip->y_descr[ycoord].base;
  lco = MAKE_AREA(chip, xcoord, ycoord);
  off = MAKE_RELATIVE(chip, x_off, y_off);
  return MAKE_NSITE(lco, off);
}

static inline unsigned
linear_global(const chip_descr_t *chip, nsite_ref_t site) {
  const unsigned rel = RELATIVE_COORD(site);
  return GET_RELATIVE_Y(chip, rel) * chip->awidth + GET_RELATIVE_X(chip, rel);
}

static inline const nsite_area_t *
area_of_site(const chip_descr_t *chip,
	     const nsite_ref_t site) {
  return &chip->area[linear_global(chip, site)];
}

/**
 * Site properties accessors, replace the csite_descr_t structure
 */
static inline site_t
relative_coords(const chip_descr_t *chip, const nsite_ref_t site) {
  const unsigned lco = RELATIVE_COORD(site);
  site_t rel = { .x = GET_RELATIVE_X(chip, lco),
		 .y = GET_RELATIVE_Y(chip, lco) };
  (void) chip;
  return rel;
}

static inline site_t
area_coords(const chip_descr_t *chip, const nsite_ref_t site) {
  const unsigned area = AREA_COORD(site);
  site_t glob = { .x = GET_AREA_X(chip, area),
		  .y = GET_AREA_Y(chip, area) };
  (void) chip;
  return glob;
}

/*
 * Have these directly a site_t, and see how this fares. Could be
 * faster. In particulare, we could use a pseudo-simd by compacting the
 * site_t into only one guint32.
 */

static inline site_t
global_coords(const chip_descr_t *chip, const nsite_ref_t site) {
  const site_t area = area_coords(chip, site);
  const site_t rel = relative_coords(chip, site);
  return add_sites(area, rel);
}

static inline site_t
local_coords(const chip_descr_t *chip, const nsite_ref_t site) {
  const nsite_area_t *area = area_of_site(chip, site);
  const site_t rel = relative_coords(chip, site);
  return add_sites(area->type_base, rel);
}

static inline site_type_t
type_of_site(const chip_descr_t *chip, const nsite_ref_t site) {
  const nsite_area_t *area = area_of_site(chip, site);
  return area->type;
}

/*
static nsite_ref_t
nsite_of_site(const chip_descr_t *chip,
	      const site_ref_t site) {
  const unsigned width = chip->width;
  const unsigned offset = site;
  const unsigned x = offset % width;
  const unsigned y = offset / width;
  return nsite_of_global(chip, x, y);
}

static site_ref_t
site_of_nsite(const chip_descr_t *chip,
	      const nsite_ref_t site) {
  const site_t scoords = global_coords(chip, site);
  return chip->width * scoords.y + scoords.x;
}
*/

/*
 * Old compatibility interface
 */

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
get_site_ref(const chip_descr_t *chip,
	     const csite_descr_t *site) {
  return (site - chip->data);
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
int snprint_slice(gchar *buf, size_t buf_len, const chip_descr_t *chip,
		  const csite_descr_t *site, const slice_index_t slice);
int parse_slice_simple(const gchar *buf, slice_index_t* idx);

/**
 * \brief Site string parsing function
 *
 * Converse of printing function snprint_csite: get a site_ref_t from its name
 *
 * @param chip The chip description
 * @param lookup The site string to be parsed
 * @param sref The site reference where the atom corresponding to the site pass
 * @return 0 on success (the string is found), !=0 on problem
 *
 * @see snprint_csite
 * @see parse_wire_simple
 */

int parse_site_simple(const chip_descr_t *chip,
		      site_ref_t* sref,
		      const gchar *lookup);

int parse_site_complex(const chip_descr_t *chip,
		       site_ref_t* sref,
		       const gchar *lookup);


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
