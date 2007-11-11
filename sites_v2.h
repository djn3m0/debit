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

#ifndef _SITES_V2_H
#define _SITES_V2_H

typedef enum _switch_type {
  SW_NONE = 0, SW_CLB,
  SW_TTERM, SW_LTERM, SW_BTERM, SW_RTERM,
  SW_TLTERM, SW_LTTERM, SW_LBTERM, SW_BLTERM,
  SW_BRTERM, SW_RBTERM, SW_RTTERM, SW_TRTERM,
  SW_TTERMBRAM, SW_BTERMBRAM,
  SW_TIOI, SW_LIOI, SW_BIOI, SW_RIOI,
  SW_TIOIBRAM, SW_BIOIBRAM,
  SW_BRAM, SW_BM,
  SW_TL, SW_BL, SW_BR, SW_TR,
  SW_M,
  SW_CLKT, SW_CLKB, SW_GCLKC, SW_GCLKH, SW_GCLKHBRAM,
  NR_SWITCH_TYPE,
} __attribute__((packed)) switch_type_t;

typedef enum _site_type {
  SITE_TYPE_NEUTRAL = 0,
  CLB,
  TTERM, LTERM, BTERM, RTERM,
  TLTERM = 6, LTTERM, LBTERM, BLTERM, BRTERM, RBTERM, RTTERM, TRTERM,
  TTERMBRAM = 14, BTERMBRAM,
  TIOI = 16, LIOI, BIOI, RIOI,
  TIOIBRAM = 20, BIOIBRAM,
  BRAM = 22, BM,
  TL = 24, BL, BR, TR,
  M = 28,
  CLKT = 29, CLKB, GCLKC, GCLKH, GCLKHBRAM,
  NR_SITE_TYPE,
} __attribute__((packed)) site_type_t;

static inline
switch_type_t sw_of_type(const site_type_t site) {
  return (switch_type_t) site;
}

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
  ZERO, ONE, TWO, THREE,
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
  /* these are used too */
  O0, O1, O2, O3, O4, O5, O6, O7,
  I0, I1, I2, I3, I4, I5, I6, I7,
  /* And these again */
  IQ1, IQ2,
  SHIFTIN, SHIFTOUT, SR,
  TBUF, TBUS,
  TI, TOUT, TS,
  VCC_PINWIRE,
  WF1_PINWIRE, WF2_PINWIRE, WF3_PINWIRE, WF4_PINWIRE,
  WG1_PINWIRE, WG2_PINWIRE, WG3_PINWIRE, WG4_PINWIRE,
  X, XB, XQ,
  Y, YB, YQ,
  LOGIC,
  NR_WIRE_TYPE,
} __attribute__((packed)) wire_type_t;

#endif /* _SITES_V2_H */
