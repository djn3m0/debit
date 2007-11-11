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

#endif /* _HAS_VIRTEX2_CONFIG_H */
