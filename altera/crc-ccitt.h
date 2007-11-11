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
 * CRC-CCITT Fast implementation ripped from the Linux kernel, licence
 * is GPL v2. See crc-ccitt.c file for copyrights, or the linux kernel
 * source. To be replaced with a self-hosted implementation in codes/
 */

#ifndef _CRC_CCITT_H
#define _CRC_CCITT_H
#include <stdint.h>

extern uint16_t const crc_ccitt_table[256];

extern uint16_t crc_ccitt(uint16_t crc, const uint8_t *buffer, size_t len);

static inline uint16_t crc_ccitt_byte(uint16_t crc, const uint8_t c)
{
	return (crc >> 8) ^ crc_ccitt_table[(crc ^ c) & 0xff];
}

#endif /* _CRC_CCITT_H */
