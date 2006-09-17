/*
 * Copyright (C) 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
 */

/*
 * Bitstream header parsing
 */

#include <glib.h>
#include "debitlog.h"

#define FIRST_OPTION_OFFSET 11

enum option_type {
  FILENAME = 0x61,
  DEVICE_TYPE,
  BUILD_DATE,
  BUILD_TIME,
  CODE,
};

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

typedef struct header_option header_option_t;
typedef struct synchro_option synchro_option_t;

static inline
header_option_t *next_option(const header_option_t *data) {
  int code = data->code;
  gint len = GINT16_FROM_BE(data->length);
  /* TODO: case on type, more info, then move this to parse_option */
  debit_log(L_HEADER, "Option code %i, length %i", code, len);
  debit_log(L_HEADER, "data: %.*s",len,data->payload);
  return (void *)&data->payload[len];
}

int parse_header(const gchar *buf, const size_t buf_len);
