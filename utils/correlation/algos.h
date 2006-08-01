/*
 * (C) Copyright 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
 * All rights reserved.
 *
 */

#ifndef _HAS_ALGOS_H
#define _HAS_ALGOS_H

typedef enum core_status {
  STATUS_NIL = 0,
  STATUS_NOTALONE,
  STATUS_ISOLATED,
} core_status_t;

core_status_t
isolate_bit_core(const state_t *state, const alldata_t *dat, const unsigned bit);

void do_all_pips(const pip_db_t *pipdb, alldata_t *dat);
void do_filtered_pips(const pip_db_t *pipdb, alldata_t *dat,
		      const char *start, const char *end);

#endif /* _HAS_ALGOS_H */
