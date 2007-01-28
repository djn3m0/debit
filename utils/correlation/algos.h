/*
 * (C) Copyright 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
 * All rights reserved.
 *
 */

#ifndef _HAS_ALGOS_H
#define _HAS_ALGOS_H

void do_all_pips(const pip_db_t *pipdb, alldata_t *dat);
void do_all_pips_thorough(const pip_db_t *pipdb, alldata_t *dat, int iterate);
void do_all_pips_internal(const pip_db_t *pipdb, alldata_t *dat, int iterate);


#endif /* _HAS_ALGOS_H */
