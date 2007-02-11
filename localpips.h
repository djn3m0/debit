/*
 * (C) Copyright 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
 * All rights reserved.
 */

#ifndef _LOCALPIPS_H
#define _LOCALPIPS_H

#include "bitstream_parser.h"
#include "wiring.h"
#include "sites.h"

/** \file */

typedef enum
  {
    /* log flags */
    PIP_LOG_DATA             = 1 << G_LOG_LEVEL_USER_SHIFT,
  } PipLevelFlags;

/** pip database opaque type
 *
 * This is an abstract view of the pip database for a chip.
 */

#ifdef __COMPILED_PIPSDB

#include "data/pips_compiled_common.h"

typedef struct pip_db {
  const pipdb_control_t *memorydb;
  wire_db_t *wiredb;
} pip_db_t;

#else /* __COMPILED_PIPSDB */

typedef struct pip_db {
  GNode *memorydb[NR_SITE_TYPE];
  wire_db_t *wiredb;
} pip_db_t;

#endif /* __COMPILED_PIPSDB */

typedef struct pip_read {
  gboolean connected;
  wire_atom_t origin;
  /* NB: could also be separated */
  GNode *connect;
} pip_read_t;

/* This stucture contains the normal form of the bitstream, fully
   unrolled into memory. It is an array of pip_read structure, which
   have their location implicitly referring to their (site, destination
   wire), with the associated input wire. Please note that the
   "connected" boolean could be reduced to a mere special wire_atom_t --
   for instance the value zero.

   Sorting in the array: sites, then wires. Seems the most natural at
   first sight; however wires, sites would probably allow for more data
   locality when iterating over sites in the innermost loops.
 */
typedef struct _pip_parsed {
  pip_read_t *bitpips;
} pip_parsed_t;

/* rather that having a full structure, we can record the pip
   completely and sort along the destination wire. This could be much
   more cache-friendly, as the data can be dense, and the cost is
   be a dichotomy during the lookup. To be benchmarked.
   However, this structure is only good for read-only operations;
   modifying it is a real pita.
*/

typedef struct _pip_parsed_dense {
  pip_t *bitpips;
  unsigned *site_index;
} pip_parsed_dense_t;

pip_db_t *get_pipdb(const gchar *datadir);
void free_pipdb(pip_db_t *pipdb);


/* utility functions */

/* This should be benchmarked and run as fast as humanly possible */
pip_parsed_dense_t *
pips_of_bitstream(const pip_db_t *pipdb, const chip_descr_t *chipdb,
		  const bitstream_parsed_t *bitstream);
void free_pipdat(pip_parsed_dense_t *pipdat);

pip_t *pips_of_site(const pip_db_t *pipdb,
		    const bitstream_parsed_t *bitstream,
		    const csite_descr_t *site,
		    gsize *size);

pip_t *pips_of_site_dense(const pip_parsed_dense_t *dat,
			  const site_ref_t site,
			  gsize *size);

gboolean
get_interconnect_startpoint(const pip_parsed_dense_t *pipdat,
			    wire_atom_t *wire,
			    const wire_atom_t orig,
			    const site_ref_t site);

typedef void (*bitpip_iterator_t)(gpointer, wire_atom_t, wire_atom_t, site_ref_t);

void
iterate_over_bitpips(const pip_parsed_dense_t *pipdat,
		     const chip_descr_t *chip,
		     bitpip_iterator_t fun, gpointer data);

#endif /* _LOCALPIPS_H */
