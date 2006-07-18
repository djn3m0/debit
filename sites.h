/*
 * (C) Copyright 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
 * All rights reserved.
 */

#ifndef _SITES_H
#define _SITES_H

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

typedef struct _site_details {
  /* coordinates of the site on the global grid */
  site_t global_coord;
  /* type-local coordinates */
  site_type_t type;
  site_t type_coord;
  /* associated pips, if any */
  gsize npips;
  pip_t *pips;
} __attribute__((packed)) site_details_t;

#endif /* _SITES_H */

