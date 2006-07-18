/*
 * (C) Copyright 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
 * All rights reserved.
 */

#ifndef _HAS_WIRING_SIMPLE_H
#define _HAS_WIRING_SIMPLE_H

#include <glib.h>
#include "inttypes.h"

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


/* and then the wire num, a simple uint16_t */
#define WIRE_NUM_NEUTRAL 32

typedef struct _wire {
  wire_type_t type;
  wire_direction_t direction;
  wire_situation_t situation;
  uint16_t num;
} __attribute__((packed)) wire_t;

/*
 * Wires, new way of doing things
 */

typedef guint16 wire_atom_t;

typedef struct _wire_simple {
  gint8 dx;
  gint8 dy;
  wire_atom_t ep;
} wire_simple_t;

typedef struct _wire_db {
  gsize dblen;
  /* series of arrays */
  wire_simple_t *wires;
  const gchar **names;
  wire_t *details;
  /* Merge with the localpips gstringchunk here */
  GStringChunk *wirenames;
} wire_db_t;

static inline
const gchar *wire_name(const wire_db_t *db, const wire_atom_t wire) {
  return (db->names[wire]);
}

static inline
wire_simple_t wire_val(const wire_db_t *db, const wire_atom_t wire) {
  return (db->wires[wire]);
}

static inline
wire_t *get_wire(const wire_db_t *db, const wire_atom_t wire) {
  return (&db->details[wire]);
}

typedef struct _pip {
  wire_atom_t source;
  wire_atom_t target;
} __attribute__((packed)) pip_t;

gint parse_wire_simple(const wire_db_t *, wire_atom_t*, const gchar *);

wire_db_t *get_wiredb(void);
void free_wiredb(wire_db_t *wires);

#endif /* _HAS_WIRING_SIMPLE_H */
