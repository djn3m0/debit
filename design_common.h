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
