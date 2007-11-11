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

#ifndef _HAS_ALGOS_H
#define _HAS_ALGOS_H

void do_all_pips(const pip_db_t *pipdb, alldata_t *dat);
void do_all_pips_thorough(const pip_db_t *pipdb, alldata_t *dat, int iterate);
void do_all_pips_internal(const pip_db_t *pipdb, alldata_t *dat, int iterate);
void dump_pips_db(const pip_db_t *pipdb);

#endif /* _HAS_ALGOS_H */
