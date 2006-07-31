/*
 * (C) Copyright 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
 * All rights reserved.
 *
 */

#include <string.h>
#include <glib.h>
#include "virtex2_config.h"

typedef struct state {
  /* Maybe we'd better represent this loosely */
  unsigned char *known_data;
  /* Full data representation */
  unsigned char *unknown_data;
} state_t;

/* state manipulation routines */
static inline void
zero_lut(site_config_t *site) {
  /* zero out the LUT data in mna 2,3 */
  memset(&site->mna[1], 0, sizeof(site_descr_t));
  memset(&site->mna[2], 0, sizeof(site_descr_t));
}

static inline int
alloc_state(state_t *to, size_t len, size_t ulen) {
  to->known_data = g_new0(unsigned char,len);
  to->unknown_data = g_new0(unsigned char,ulen);
  return 0;
}

static inline void
init_state(state_t *to, size_t len, size_t ulen) {
  memset(to->known_data, 0xFF, len);
  memset(to->unknown_data, 0xFF, ulen);
  zero_lut((site_config_t *)to->known_data);
  zero_lut((site_config_t *)to->unknown_data);
}

static inline void
release_state(state_t *to) {
  g_free(to->known_data);
  g_free(to->unknown_data);
}
