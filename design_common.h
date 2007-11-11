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
#ifndef _DESIGN_COMMON_H
#define _DESIGN_COMMON_H

/*
 * The frame index is a four-way lookup table. We have chosen for now to use
 * a two-way lookup table to index the frames internally, redoing most
 * of the computation for the v4.
 */

static inline
const gchar **get_frame_loc(const bitstream_parsed_t *parsed,
			    const guint type,
			    const guint row,
			    const guint top,
			    const guint index,
			    const guint frame) {
  const chip_struct_t *chip_struct = parsed->chip_struct;
  const unsigned rowcount = chip_struct->row_count;
  const unsigned framecount = chip_struct->frame_count[type];
  const unsigned *col_count = chip_struct->col_count;
  g_assert(type < V__NB_CFG);
  g_assert(index < type_col_count(col_count, type));
  g_assert(row < rowcount);
  g_assert(frame < framecount);
  (void) col_count;

  /* This is a double-lookup method */
  return &parsed->frames[type][ framecount * ((index * 2 + top) * rowcount + row)
				+ frame ];
}

static inline
const gchar *get_frame(const bitstream_parsed_t *parsed,
		       const guint type,
		       const guint row,
		       const guint top,
		       const guint index,
		       const guint frame) {
  const gchar *frameptr = *get_frame_loc(parsed, type, row, top, index, frame);
  g_assert(frameptr != NULL);
  return frameptr;
}

#endif /* _DESIGN_COMMON_H */
