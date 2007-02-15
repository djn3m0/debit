/*
 * (C) Copyright 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
 * All rights reserved.
 */

#ifndef _SITES_V5_H
#define _SITES_V5_H

typedef enum _switch_type {
  SW_NONE = 0,
  SW_INT, SW_CLB,
  SW_IOB, SW_DSP48,
  SW_BRAM, NR_SWITCH_TYPE,
} __attribute__((packed)) switch_type_t;

typedef enum _site_type {
  SITE_TYPE_NEUTRAL = 0,
  IOB, CLB, DSP48,
  GCLKC, BRAM,
  NR_SITE_TYPE,
} __attribute__((packed)) site_type_t;

#define SIMPLE_SITE(x) [x] = (1ULL << SW_ ## x) | (1ULL << SW_INT)
static const uint64_t sw_of_type[NR_SITE_TYPE] = {
  [SITE_TYPE_NEUTRAL] = 0,
  SIMPLE_SITE(CLB),
  SIMPLE_SITE(IOB),
  SIMPLE_SITE(DSP48),
};
#undef SIMPLE_SITE

typedef enum _wire_direction {
  WIRE_DIRECTION_NEUTRAL = 0,
  N, WN, NW,
  W, WS, SW,
  S, SE, ES,
  E, EN, NE,
  DN, UP, // global clock
  NR_WIRE_DIRECTION,
} __attribute__((packed)) wire_direction_t;

typedef enum _wire_situation {
  WIRE_SITUATION_NEUTRAL = 0,
  BEG, A, B, MID, C, D, END,
  NR_WIRE_SITUATION,
} __attribute__((packed)) wire_situation_t;

typedef enum _wire_type {
  WIRE_TYPE_NEUTRAL = 0,
  DOUBLE, HEX,
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
  NR_WIRE_TYPE,
} __attribute__((packed)) wire_type_t;

#endif /* _SITES_V5_H */
