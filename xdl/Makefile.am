AM_CFLAGS	= -Wall -Wextra -Wunused -O3 -g

AM_YFLAGS = -d
AM_LFLAGS = -Cfe --perf-report --verbose
bin_PROGRAMS    = xdl2bit
xdl2bit_SOURCES = xdl2bit.c xdl_lexer.l xdl_parser.y parser.h

# The generated header from flex can be useful
BUILT_SOURCES = xdl_parser.h
