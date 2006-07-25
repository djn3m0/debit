/*
 * Copyright (C) 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
 */

#include <glib.h>

#include "bitstream.h"
#include "bitarray.h"


/* the meat */

/* Altera bitstream files are a
   three-dimentional array.
   First dimention is X, then slicing, then Y
*/

#define EP35_X_SITES 66

typedef struct _geo_stride {
  unsigned offset;
  unsigned length;
} geo_stride_t;

static const geo_stride_t x_offsets_descr[] = {
  /* column 0 is IO */
  { .offset = 34,
    .length = 1 },
  { .offset = 35,
    .length = 12 },
  /* don't know what this is */
  { .offset = 166,
    .length = 1 },
  { .offset = 35,
    .length = 1 },
  /* small thing: interconnect site ? */
  { .offset = 4,
    .length = 1 },
  /* standard things */
  { .offset = 35,
    .length = 10 },
  /* again big in the middle */
  { .offset = 166,
    .length = 1 },
  /* event 49->50 */
  { .offset = 35,
    .length = 10, },
  { .offset = 4,
    .length = 1, },
  { .offset = 35,
    .length = 1, },
  { .offset = 166,
    .length = 1 },
  { .offset = 35,
    .length = 12, },
  { .offset = 34,
    .length = 1, },
  /* NULL-terminate it */
  { .offset = 0,
    .length = 0, },
};

static inline unsigned
width_total(const geo_stride_t *strides) {
  unsigned offset_tot = 0;
  unsigned length;

  do {
    length = strides->length;
    offset_tot += length;
    strides++;
  } while (length != 0);
  return offset_tot;
}

static inline unsigned
offset_total(const geo_stride_t *strides) {
  unsigned offset_tot = 0;
  unsigned length, offset;

  do {
    length = strides->length;
    offset = strides->offset;
    offset_tot += length * offset;
    strides++;
  } while (length != 0);
  return offset_tot;
}

/* accumulate the x_offsets_descr and build corresponding
   offset array */
static unsigned *
offset_array(const geo_stride_t *strides) {
  unsigned total = width_total(strides);
  unsigned *array = g_new0(unsigned, total);
  unsigned offset = 0;
  unsigned steps, stepping;

  do {
    unsigned i;

    steps = strides->length;
    stepping = strides->offset;

    for (i = 0; i < steps; i++) {
      array[i] = offset;
      offset += stepping;
    }

    array += steps;

  } while (steps != 0);

  return array;
}

/* the absolute coordinate of (0,0,0) -- well, a rought guess actually */
#define BASE_BITS

typedef struct _coord_descr_t {
  unsigned max;
  unsigned offset;
} coord_descr_t;

typedef enum coords_types {
  Y = 0, SLICE, X, COORD_NUM,
} coord_type_t;

static const coord_descr_t coords[COORD_NUM] = {
  [Y] = { .max = 35, .offset = 70 * 2701 },
  [SLICE] = { .max = 69, .offset = 2701 /*could be computed from above
					  too, this is the offset_total*/ },
  /* For X, a bit complicated, see above */
  [X] = { .max = 65, .offset = 0 /* computed normally from above */},
};

static inline unsigned
get_bit_offset(const altera_bitstream_t *bitstream,
	       unsigned y, unsigned slice, unsigned x) {
  /* start from beginning */
  unsigned y_offset = coords[Y].offset * y;
  unsigned x_offset = bitstream->xoffsets[x];
  unsigned slice_offset = coords[SLICE].offset * x;

  return BASE_BITS - y_offset - x_offset - slice_offset;
}

static inline unsigned
get_chunk_len(const altera_bitstream_t *bitstream, unsigned x) {
  (void) bitstream;
  return x_offsets_descr[x].offset;
}

static inline gboolean
get_bit(const altera_bitstream_t *bitstream,
	unsigned x, unsigned y, unsigned slice, unsigned offset) {
  bitarray_t *bitarray = bitstream->bitarray;
  unsigned bitpos = get_bit_offset(bitstream, y, slice, x) + offset;

  /* Warn if we're looking */
  g_assert(offset < get_chunk_len(bitstream, x));
  return bitarray_is_set(bitarray, bitpos) ? TRUE : FALSE;
}

/*
 * XXX will need more than that at some point
 * Get bits in [start, end[
 */

static inline
guint32 get_chunk(const altera_bitstream_t *bitstream,
		  unsigned x, unsigned y, unsigned slice,
		  unsigned start, unsigned end) {
  guint32 res = 0;
  unsigned len = end - start;
  unsigned i;

  for (i = 0; i < len; i++)
    if (get_bit(bitstream,x,y,slice,start+i))
      res |= 1 << i;

  return res;
}

/* the truth table really is nasty */

static const char xor_mask[4] = {
  [0] = 0, [1] = 0, [2] = 0, [3] = 0
};

/* XXX needs reordering */
static inline
guint16 get_truth_table(const altera_bitstream_t *bitstream,
			unsigned x, unsigned y, unsigned N) {
  guint32 res = 0;
  unsigned i;

  /* the table is divided into four things of four bits */
  /* there may be a xor mask to apply ! */
  for (i = 0; i < 4; i++) {
    guint32 table = get_chunk(bitstream, x, y, (N/2) * 4 + 3, 0, 4);
    res |= (table ^ xor_mask[i]) << (4*i);
  }

  return res;
}

static inline bitarray_t *
get_lab_data() {
  /* iterate over all slices in the lab, and get all data */

  /* this is more difficult, it requires guint64 */
  return NULL;
}

/* high-level interface */

altera_bitstream_t *
parse_bitstream(const gchar *filename) {
  /* get the bitstream into a bitarray */
  GError *error = NULL;
  GMappedFile *file = NULL;
  altera_bitstream_t *bit;

  bit = g_new0(altera_bitstream_t, 1);

  file = g_mapped_file_new (filename, FALSE, &error);
  bit->file = file;

  if (error != NULL) {
    //debit_log(L_BITSTREAM,"could not map file %s: %s",filename,error->message);
    g_error_free (error);
    goto out_free_dest;
  }

  /* load the file into the bitarray */
  bit->bitarray = bitarray_create_data(g_mapped_file_get_contents (file),
				       8 * g_mapped_file_get_length (file));

  /* then do mighty things with it ! */
  bit->xoffsets = offset_array(x_offsets_descr);

 out_free_dest:
  g_free (bit);
  return NULL;
}

void free_bitstream(altera_bitstream_t *bitstream) {
  g_mapped_file_free(bitstream->file);
  g_free(bitstream);
}
