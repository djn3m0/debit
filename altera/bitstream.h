
#include <glib.h>
#include "bitarray.h"

typedef struct _altera_bitstream_t {
  bitarray_t *bitarray;
  unsigned *xoffsets;
  unsigned base_offset;
  GMappedFile *file;
} altera_bitstream_t;

altera_bitstream_t *
parse_bitstream(const gchar *file);

void free_bitstream(altera_bitstream_t *);
