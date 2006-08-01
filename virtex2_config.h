/*
 * Copyright (C) 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
 */

/*
 * Binary structure describing the Frame layout of the Virtex-II
 * They may need to be adapted a little for genericity, but KISS for
 * now, and see what can be shared as we go along.
 */

#ifndef _HAS_VIRTEX2_CONFIG_H
#define _HAS_VIRTEX2_CONFIG_H

#include <inttypes.h>

typedef struct slice_descr {
	uint16_t Feqn;
	uint8_t  unk1; /* probably some config for the bunch of things
			* in the middle, or the lut/rom/etc config */
	uint16_t Geqn; /* inferred, not 100% sure */
} __attribute__((packed)) slice_descr_t;

typedef struct site_descr {
	slice_descr_t slices[2];
} __attribute__((packed)) site_descr_t;

#define SITE_PER_COL 56

#define SLICE_OFF 12
#define SLICE_TAIL 12
#define SLICE_PER_COL (2*SITE_PER_COL)

typedef struct slice_frame {
	uint8_t unk1[SLICE_OFF];
	slice_descr_t slices[SLICE_PER_COL];
	uint8_t unk2[SLICE_TAIL];
} __attribute__((packed)) slice_frame_t;

typedef struct site_frame {
	uint8_t unk1[SLICE_OFF];
	site_descr_t sites[SITE_PER_COL];
	uint8_t unk2[SLICE_TAIL];
} __attribute__((packed)) site_frame_t;

typedef struct site_config {
	site_descr_t mna[22];
} __attribute__((packed)) site_config_t;

#endif /* _HAS_VIRTEX2_CONFIG_H */
