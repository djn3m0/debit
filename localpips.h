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

typedef struct pip_db {
  GNode *memorydb[NR_SITE_TYPE];
  wire_db_t *wiredb;
} pip_db_t;

pip_db_t *get_pipdb(const gchar *datadir);
void free_pipdb(pip_db_t *pipdb);


/* utility functions */
pip_t *pips_of_site(const pip_db_t *pipdb,
		    const bitstream_parsed_t *bitstream,
		    const csite_descr_t *site,
		    gsize *size);

gboolean get_wire_startpoint(const pip_db_t *pipdb,
			     const chip_descr_t *chipdb,
			     sited_wire_t *wire,
			     const sited_wire_t *orig);

gboolean
get_interconnect_startpoint(const pip_db_t *pipdb,
			    const bitstream_parsed_t *bitstream,
			    sited_wire_t *wire,
			    const sited_wire_t *orig);

typedef void (*bitpip_iterator_t)(gpointer, wire_atom_t, wire_atom_t, site_ref_t);

void
iterate_over_bitpips(const pip_db_t *pipdb,
		     const bitstream_parsed_t *bitstream,
		     const site_ref_t site,
		     bitpip_iterator_t fun,
		     gpointer data);

#endif /* _LOCALPIPS_H */
