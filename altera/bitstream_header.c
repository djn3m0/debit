/*
 * Parser for the bitstream header
 */

#include <glib.h>
#include <stdint.h>

#include "bitstream.h"
#include "bitstream_header.h"

typedef enum option_code {
  NEUTRAL = 0,
  BUILDTOOL = 1,
  CHIPTYPE = 2,
  NAME = 3,
  CRC = 8,
  BITSTREAM = 17,
  PACKAGE = 18,
  UNKNOWN = 19,
} option_code_t;

#define PACKED __attribute__((packed))

/* we need the packed as this is an external structure */
/* fuck altera. They know nothing about alignment issues. Probably some
   inter mistake again */
typedef struct _bitstream_option {
  uint16_t option; /* le */
  uint32_t length; /* le */
  char data[];
} PACKED bitstream_option_t;

static inline
uint32_t get_32(const char *data) {
  return *((uint32_t *)data);
}

static inline const bitstream_option_t *
parse_option(altera_bitstream_t *altera,
	     option_code_t *rcode,
	     const bitstream_option_t *opt) {
  unsigned code = GUINT16_FROM_LE(opt->option);
  guint32 length = GUINT32_FROM_LE(opt->length);
  const char *data = opt->data;

  g_print("option code %i, length %i\n", code, length);

  switch(code) {
  case BUILDTOOL:
    g_print("This bitstream build courtesy of %.*s. We love you !\n", length, data);
    break;
  case CHIPTYPE:
    g_print("Chip is a %.*s\n", length, data);
    break;
  case NAME:
    g_print("This bitstream's joyfull nickname is %.*s\n", length, data);
    break;
  case BITSTREAM:
    g_print("Got the bitstream data. The meat. What you want. (length %i)\n", length);
    altera->bitdata = data;
    altera->bitlength = length;
    break;
  case PACKAGE:
    {
      guint32 id = GUINT32_FROM_LE(get_32(data));
      g_print("Something, maybe the package is %04x\n", id);
    }
    break;
  case CRC:
    {
      guint32 crc = GUINT32_FROM_LE(get_32(data));
      g_print("CRC for the bitstream is %08x\n", crc);
    }
    break;
  default:
    g_print("Unknown option code. Cheers ! And do not forget to contact me.\n");
  }

  *rcode = code;
  return (const void *)&data[length];
}

#define SOF_MAGIC 0x534f4600

int
parse_bitstream_structure(altera_bitstream_t *bitstream,
			  const gchar *buf, const size_t buf_len) {
  const bitstream_option_t *current;
  const bitstream_option_t *last_option = (void *) (buf + buf_len);
  const gchar *read = buf;
  option_code_t code = NEUTRAL;

  /* recognize SOF magic */
  if (GUINT32_FROM_BE(get_32(read)) != SOF_MAGIC) {
    g_warning("This does not look like a .sof file");
    return -1;
  }

  /* seek to the first option past SOF-- I need more bitstream examples to
     reverse-engeneer the very beginning of the file */

  read += 3 * sizeof(uint32_t);
  current = (void *)read;

  while (current < last_option) {
    current = parse_option(bitstream, &code, current);
  }

  /* yeah, there can be no error with such rock-solid code */
  return 0;
}
