/*
 * Copyright (C) 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
 */

#include <glib.h>

#include "bitstream_header.h"
#include "bitstream.h"
#include "bitarray.h"


/* the meat */

/* Altera bitstream files are a
   three-dimentional array.
   First dimention is X, then slicing, then Y
*/

/* 0 - 65 */
#define EP35_X_SITES 66
/* 0 - 36, but we restrict here to 1-35
   The border causes problems and we don't want it */
#define EP35_Y_SITES 35
#define EP35_LAB_SITES 32

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
  /* don't know what this is, X = 13 */
  { .offset = 166,
    .length = 1 },
  { .offset = 35,
    .length = 1 },
  /* small thing: interconnect site ? */
  { .offset = 4 + 35,
    .length = 1 },
  /* standard thing */
  { .offset = 35,
    .length = 10 },
  /* another @X = 26 */
  { .offset = 166,
    .length = 1 },
  { .offset = 35,
    .length = 12 },
  /* again strange, @X = 39 */
  { .offset = 27,
    .length = 1 },
  /* event 49->50 */
  { .offset = 35,
    .length = 10, },
  { .offset = 4 + 35,
    .length = 1, },
  { .offset = 35,
    .length = 1, },
  /* last strange @X = 52 */
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

static inline void
check_consistency(const altera_bitstream_t *bit) {
  unsigned offset = offset_total(x_offsets_descr);
/*   unsigned nx = width_total(x_offsets_descr); */
/*   g_print("offset total is %i for %i sites\n", offset, nx); */
  g_assert( offset == 2701 );
}

/* accumulate the x_offsets_descr and build corresponding
   offset array */
static unsigned *
offset_array(const geo_stride_t *strides) {
  unsigned total = width_total(strides);
  unsigned *array = g_new(unsigned, total);
  unsigned *posarray = array;
  unsigned offset = 0;
  unsigned steps, stepping;

  do {
    unsigned i;

    steps = strides->length;
    stepping = strides->offset;

    for (i = 0; i < steps; i++) {
      posarray[i] = offset;
      offset += stepping;
    }

    posarray += steps;
    strides++;

  } while (steps != 0);

  return array;
}

/* the absolute coordinate of (1,1,30) -- adjusted for various offsets */
#define BASE_BITS (6479737+34-1*2701)

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

/*
  NB: slices seem ordered in a bizarre fashion. Reordering them would
  take the following permutation (which is expressed in pure code
  instead of data in get_truth_table)
  there would be something like this
  1 extremity
  32 (4*8) standard, containing luts
  4 central
  32 (4*8) standard, containing luts
  1 extremity
 */

static inline unsigned
get_bit_offset(const altera_bitstream_t *bitstream,
	       unsigned y, unsigned slice, unsigned x) {
  /* start from beginning */
  unsigned y_offset = coords[Y].offset * (y-1);
  unsigned x_offset = bitstream->xoffsets[x];
  unsigned slice_offset = coords[SLICE].offset * slice;

  /* yes. Don't ask. Another intern. */
  unsigned ret = BASE_BITS - y_offset - x_offset + slice_offset;

/*   g_print("offset is %i, y %i, x %i, slice %i\n", */
/* 	  ret, y_offset, x_offset, slice_offset); */
  return ret;
}

static inline unsigned
get_chunk_len(const altera_bitstream_t *bitstream, unsigned x) {
  return (bitstream->xoffsets[x+1] - bitstream->xoffsets[x]);
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
  bitarray_t *bitarray = bitstream->bitarray;
  unsigned basebit = get_bit_offset(bitstream,y,slice,x) + start;
  guint32 res = 0;
  unsigned i;

  for (i = start; i < end; i++)
    if (bitarray_is_set(bitarray,basebit++))
      res |= 1 << i;

  return res;
}

static inline
guint16 get_truth_table(const altera_bitstream_t *bitstream,
			unsigned x, unsigned y, unsigned N) {
  guint16 res = 0;
  unsigned slice, i;

  g_assert(N % 2 == 0);
/*   g_print("getting truth table for (%i, %i, %i)\n", */
/* 	  x, y , N); */

  /* lut ordering is a bit weird (see above for slice details) */
  if (N < 16) {
    /* or test 16 bit */
    slice = 1 + 2*N;
  } else {
    slice = 69 + 2 - 4 - (N-16)*2;
  }
  /* the table is divided into four things of four bits */
  /* there may be a xor mask to apply ! */
  for (i = 0; i < 4; i++) {
    guint32 table = get_chunk(bitstream, x, y, slice + i, 0, 4);
    res |= table << (4*i);
  }

  return ~res;
}

static inline bitarray_t *
get_lab_data() {
  /* iterate over all slices in the lab, and get all data */

  /* this is more difficult, it requires guint64 */
  return NULL;
}

/* high-level interface */
void
dump_lut_tables(const altera_bitstream_t *bitstream) {
  unsigned x;
  /* iterate over all lut tables. This is a bit tricky, as some sites
     are not lut-enabled */
  for ( x = 1; x < EP35_X_SITES-1; x++)
    /* if we're of the right type. This is a rought filter */
    if (get_chunk_len(bitstream,x) == 35) {
      unsigned y;
      for ( y = 1; y <= EP35_Y_SITES; y++) {
	unsigned n;
	for ( n = 0; n < EP35_LAB_SITES; n+=2) {
	  guint16 table = get_truth_table(bitstream,x,y,n);
	  if (table != 0 && table != 0xffff)
	    g_print("(%i,%i,%i) is %04x\n",x,y,n,table);
	}
      }
    } else {
      g_print("not considering x value %i\n",x);
    }
}

int
dump_raw_bit(const altera_bitstream_t *bitstream, const gchar *filename) {
  GError *error = NULL;
  gboolean ret;

  ret = g_file_set_contents(filename, bitstream->bitdata,
			    bitstream->bitlength, &error);

  if (ret)
    return 0;

  g_print("could not dump file %s: %s",filename,error->message);
  g_error_free (error);
  return -1;
}

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
    g_warning("could not map file %s: %s",filename,error->message);
    g_error_free (error);
    goto out_free_dest;
  }

  /* grossly parse the structure */
  parse_bitstream_structure(bit, g_mapped_file_get_contents (file),
			    g_mapped_file_get_length (file));

  /* load the actual bitstream data into the bitarray */
  bit->bitarray = bitarray_create_data((char *)bit->bitdata, 8 * bit->bitlength);

  /* then do mighty things with it ! */
  bit->xoffsets = offset_array(x_offsets_descr);

  /* do some basic checks */
  check_consistency(bit);

  return bit;

 out_free_dest:
  g_free (bit);
  return NULL;
}

void free_bitstream(altera_bitstream_t *bitstream) {
  g_free(bitstream->xoffsets);
  (void) bitarray_free(bitstream->bitarray, TRUE);
  g_mapped_file_free(bitstream->file);
  g_free(bitstream);
}
