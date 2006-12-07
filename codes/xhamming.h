/*
 * Copyright (C) 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
 *
 * Xilinx hamming code implementation, as seen in virtex-4 and virtex-5
 */

#ifndef _HAS_XHAMMING_H
#define _HAS_XHAMMING_H

#include <stdint.h>

int check_hamming_frame(const char *data, const uint32_t far);

#endif /* _HAS_XHAMMING_H */
