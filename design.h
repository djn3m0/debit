/*
 * (C) Copyright 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
 * All rights reserved.
 */

#ifndef _HAS_DESIGN_H
#define _HAS_DESIGN_H

#if defined(VIRTEX2)
#include "design_v2.h"
#elif defined(VIRTEX4)
#include "design_v4.h"
#elif defined(VIRTEX5)
#include "design_v5.h"
#else
#error "You must define a type macro"
#endif

#define CHIP_STRUCT(x) ((chip_struct_t *)chip_struct)

#endif /* _HAS_DESIGN_H */
