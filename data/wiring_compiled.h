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
 * Converting a flat DB file to a standard file.
 * This is inspired by the triple-include trick from the DSO howto
 */

/*
 * For this first implementation, we mimic the current implementation
 * with three separate databases.
 */

/* This is the string array & string lookup table generation */

#include <stddef.h>
#define MSGSTRFIELD(line) MSGSTRFIELD1(line)
#define MSGSTRFIELD1(line) str##line

static const union msgstr_t {
	struct {
#define _WIRE_ENTRY(s, ...) char MSGSTRFIELD(__LINE__)[sizeof(#s)];
#include WIREDB
#undef _WIRE_ENTRY
	};
	char str[0];
} wirestr = { {
#define _WIRE_ENTRY(s, ...) #s,
#include WIREDB
#undef _WIRE_ENTRY
} };

static const unsigned int wireidx[] = {
#define _WIRE_ENTRY(s, n, ...) [n] = offsetof(union msgstr_t, MSGSTRFIELD(__LINE__)),
#include WIREDB
#undef _WIRE_ENTRY
};

const char *wirename (unsigned nr) {
	return wirestr.str + wireidx[nr];
}

/* This is the simpler fixed-size structure array generation */

static const wire_simple_t wires[] = {
#define _WIRE_ENTRY(s, n, vdx, vdy, vep, ...) [n] = { .dx = vdx, .dy = vdy, .ep = vep },
#include WIREDB
#undef _WIRE_ENTRY
};

static const wire_t details[] = {
#define _WIRE_ENTRY(s, n, vdx, vdy, vep, new_nep, new_eps, vtype, vdir, vsit) [n] = { .type = vtype, .direction = vdir, .situation = vsit },
#include WIREDB
#undef _WIRE_ENTRY
};
