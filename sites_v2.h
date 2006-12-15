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

typedef enum _wire_direction {
  N = 0, WN, NW,
  W, WS, SW,
  S, SE, ES,
  E, EN, NE,
  DN, UP, // global clock
  WIRE_DIRECTION_NEUTRAL,
  NR_WIRE_DIRECTION,
} __attribute__((packed)) wire_direction_t;

typedef enum _wire_situation {
  BEG = 0, A, B, MID, C, D, END,
  WIRE_SITUATION_NEUTRAL,
  NR_WIRE_SITUATION,
} __attribute__((packed)) wire_situation_t;

typedef enum _wire_type {
  DOUBLE = 0, HEX,
  OMUX,
  BX, BY,
  BX_PINWIRE, BY_PINWIRE,
  CE, CIN,
  CLK,
  COUT,
  DX, DY,
  F1, F2, F3, F4,
  F1_PINWIRE, F2_PINWIRE, F3_PINWIRE, F4_PINWIRE,
  F5, FX, FXINA, FXINB,
  G1, G2, G3, G4,
  G1_PINWIRE, G2_PINWIRE, G3_PINWIRE, G4_PINWIRE,
  GCLK,
  // XXX might want to replace those 4 with one type + DIR
  GCLKC_GCLKB, GCLKC_GCLKL, GCLKC_GCLKR, GCLKC_GCLKT,
  GCLKH_GCLK_B,
  GCLKH_GCLK, // with dir DN and UP
  LH, LV,
  SHIFTIN, SHIFTOUT, SR,
  TBUF, TBUS,
  TI, TOUT, TS,
  VCC_PINWIRE,
  WF1_PINWIRE, WF2_PINWIRE, WF3_PINWIRE, WF4_PINWIRE,
  WG1_PINWIRE, WG2_PINWIRE, WG3_PINWIRE, WG4_PINWIRE,
  X, XB, XQ,
  Y, YB, YQ,
  WIRE_TYPE_NEUTRAL,
  NR_WIRE_TYPE,
} __attribute__((packed)) wire_type_t;

#endif /* _SITES_V2_H */
