/*
 * (C) Copyright 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
 * All rights reserved.
 */

#ifndef _SITES_V2_H
#define _SITES_V2_H

typedef enum _site_type {
  CLB = 0,
  TTERM, LTERM, BTERM, RTERM,
  TLTERM = 5, LTTERM, LBTERM, BLTERM, BRTERM, RBTERM, RTTERM, TRTERM,
  TTERMBRAM = 13, BTERMBRAM,
  TIOI = 15, LIOI, BIOI, RIOI,
  TIOIBRAM = 19, BIOIBRAM,
  BRAM = 21, BM,
  TL = 23, BL, BR, TR,
  M = 27,
  CLKT = 28, CLKB, GCLKC, GCLKH, GCLKHBRAM,
  SITE_TYPE_NEUTRAL,
  NR_SITE_TYPE,
} __attribute__((packed)) site_type_t;

#endif /* _SITES_V2_H */
