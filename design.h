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

#ifndef _HAS_DESIGN_H
#define _HAS_DESIGN_H

#if defined(VIRTEX2)
#include "design_v2.h"
#elif defined(SPARTAN3)
#include "design_s3.h"
#elif defined(VIRTEX4)
#include "design_v4.h"
#elif defined(VIRTEX5)
#include "design_v5.h"
#else
#error "You must define a type macro"
#endif

#define CHIP_STRUCT(x) ((chip_struct_t *)chip_struct)

#endif /* _HAS_DESIGN_H */
