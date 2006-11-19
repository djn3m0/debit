/*
 * Fast implementation ripped from the Linux kernel,
 * licence is GPL v2. See .c file for copyrights.
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
