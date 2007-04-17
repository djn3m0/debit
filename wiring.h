/*
 * (C) Copyright 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
 * All rights reserved.
 */

#ifndef _HAS_WIRING_SIMPLE_H
#define _HAS_WIRING_SIMPLE_H

#include <glib.h>
#include "inttypes.h"

#include "sites.h"

/* and then the wire num, a simple uint16_t */

typedef struct _wire {
  wire_type_t type;
  wire_direction_t direction;
  wire_situation_t situation;
} __attribute__((packed)) wire_t;

/*
 * Wires, new way of doing things
 */

typedef guint16 wire_atom_t;

#define WIRE_EP_END ((wire_atom_t) -1)

typedef struct _wire_simple {
  gint8 dx;
  gint8 dy;
  wire_atom_t ep;
} __attribute__((packed)) wire_simple_t;

typedef struct _sited_wire {
  site_ref_t site;
  wire_atom_t wire;
} sited_wire_t;

typedef struct _wire_db {
  gsize dblen;
  /* series of arrays */
  const wire_simple_t *wires;
  const wire_t *details;
#ifdef __COMPILED_WIREDB
  const unsigned int *wireidx;
  const gchar *wirenames;
#else
  const gchar **names;
  GStringChunk *wirenames;
#endif
} wire_db_t;

static inline
wire_type_t
wire_type(const wire_db_t *db, const wire_atom_t wire) {
  return db->details[wire].type;
}

static inline
unsigned
wire_situation(const wire_db_t *db, const wire_atom_t wire) {
  return db->details[wire].situation;
}

#ifdef __COMPILED_WIREDB

static inline
const char *wire_name(const wire_db_t *db, const wire_atom_t wire) {
  return db->wirenames + db->wireidx[wire];
}

#else

static inline
const gchar *wire_name(const wire_db_t *db, const wire_atom_t wire) {
  return (db->names[wire]);
}

#endif

static inline
const wire_simple_t *wire_val(const wire_db_t *db, const wire_atom_t wire) {
  return (&db->wires[wire]);
}

static inline
const wire_t *get_wire(const wire_db_t *db, const wire_atom_t wire) {
  return (&db->details[wire]);
}

typedef struct _pip {
  wire_atom_t source;
  wire_atom_t target;
} __attribute__((packed)) pip_t;

typedef struct _sited_pip {
  site_ref_t site;
  pip_t pip;
} sited_pip_t;

void sprint_spip(gchar *buf, const wire_db_t *wdb, const chip_descr_t *chip,
		 const sited_pip_t *spip);

/* /\* \brief Fill a sited_wire_t from the origin component of a sited_pip_t */
/*  *\/ */
/* static inline void */
/* get_source(sited_wire_t *orig, */
/* 	   const sited_pip_t *pip) { */
/*   orig->site = pip->site; */
/*   orig->wire = pip->pip.source; */
/* } */

gint parse_wire_simple(const wire_db_t *, wire_atom_t*, const gchar *);

gboolean
get_wire_startpoint(const wire_db_t *wiredb, const chip_descr_t *chipdb,
		    site_ref_t *starget, wire_atom_t *wtarget,
		    const site_ref_t sorig, const wire_atom_t worig);

wire_db_t *get_wiredb(const gchar *datadir);
void free_wiredb(wire_db_t *wires);

#endif /* _HAS_WIRING_SIMPLE_H */
