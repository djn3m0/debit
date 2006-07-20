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

#define ATTR_UNUSED_OK __attribute__((unused))

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
		    const site_details_t *site,
		    gsize *size);

void print_pipdb(GKeyFile *pipdb);
void dump_all_pips(const pip_db_t *pipdb,
		   const bitstream_parsed_t *bitstream);

#endif /* _LOCALPIPS_H */
