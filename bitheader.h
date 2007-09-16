/*
 * Copyright (C) 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
 */

#ifndef _HAS_BITHEADER_H
#define _HAS_BITHEADER_H

/*
 * Bitstream header parsing
 */

#include <glib.h>
#include "debitlog.h"

#define FIRST_OPTION_OFFSET 11

typedef enum option_type {
  FILENAME = 'a',
  DEVICE_TYPE = 'b',
  BUILD_DATE = 'c',
  BUILD_TIME = 'd',
  CODE = 'e',
  LAST_OPTION,
} option_type_t;

struct header_option {
  guint8 code;
  guint16 length;
  gchar payload[];
} __attribute__((packed));

struct synchro_option {
  guint8  code;
  guint32 bitstream_length;
  guint32 data[];
} __attribute__((packed));

typedef struct _header_option_p {
  guint16 len;
  const char *data;
} header_option_p;

typedef struct _parsed_header {
  header_option_p options[ LAST_OPTION - FILENAME ];
} parsed_header_t;

typedef struct header_option header_option_t;
typedef struct synchro_option synchro_option_t;

static inline unsigned
get_option_len(const header_option_t *opt) {
  return GINT16_FROM_BE(opt->length);
}

static inline const header_option_p*
get_option(const parsed_header_t *header,
	   const option_type_t opt) {
  return &header->options[opt - FILENAME];
}

static inline
header_option_t *next_option(const header_option_t *data) {
  gint len = GINT16_FROM_BE(data->length);
  return (void *)&data->payload[len];
}

int parse_header(parsed_header_t *parse,
		 const gchar *buf, const size_t buf_len);

#endif /* _HAS_BITHEADER_H */
