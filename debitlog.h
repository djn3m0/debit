/*
 * Copyright Jean-Baptiste Note <jean-baptiste.note@m4x.org>
 */

#ifndef _HAS_DEBIT_DEBUG_H
#define _HAS_DEBIT_DEBUG_H

#include <glib.h>

#define ARRAY_SIZE(a) (sizeof(a)/sizeof(a[0]))

#ifndef DEBIT_DEBUG
#define DEBIT_DEBUG 2
#endif

enum {
	/* generic debug */
	L_DEBUG = (DEBIT_DEBUG > 1) * 0x0001,
	/* function tracing */
	L_FUNC = (DEBIT_DEBUG > 0) * 0x0002,

	/* Bitstream control data */
	L_BITSTREAM = (DEBIT_DEBUG > 0) * 0x0004,
	/* Pip debug data */
	L_PIPS =  (DEBIT_DEBUG > 1) * 0x0008,
	/* Wires debug */
	L_WIRES = (DEBIT_DEBUG > 1) * 0x0010,
	/* Connexity algorithms */
	L_CONNEXITY = (DEBIT_DEBUG > 1) * 0x0020,
	/* Sites debug */
	L_SITES = (DEBIT_DEBUG > 1) * 0x0040,
	/* Correlation algorithms */
	L_CORRELATE = (DEBIT_DEBUG > 1) * 0x1000,
	/* Drawing */
	L_DRAW = (DEBIT_DEBUG > 1) * 0x2000,
	/* GUI */
	L_GUI = (DEBIT_DEBUG > 1) * 0x4000,
	/* Header */
	L_HEADER = (DEBIT_DEBUG > 1) * 0x8000,
	/* Parser / lexer */
	L_PARSER = (DEBIT_DEBUG > 1) * 0x10000,
	L_LEXER = (DEBIT_DEBUG > 1) * 0x20000,
	/* Bitstream writer */
	L_WRITE = (DEBIT_DEBUG > 1) * 0x40000,
	L_ANY = 0xfffff,
};

#if DEBIT_DEBUG > 0
extern unsigned int     debit_debug;
#else
enum { debit_debug = 0 };
#endif

#if DEBIT_DEBUG > 0

#define debit_log(chan, args...) \
	do { \
		if (debit_debug & (chan)) \
			g_warning(args); \
	} while (0)

#else

#define debit_log(chan, args...)

#endif

#endif /* _HAS_DEBIT_DEBUG_H */
