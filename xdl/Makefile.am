AM_CPPFLAGS	= -DDATADIR=\"$(pkgdatadir)\" -I$(top_srcdir)
AM_CFLAGS	= -Wall -Wunused -Werror -O3 -g

AM_YFLAGS = -d
AM_LFLAGS = -Cfe --perf-report --verbose --nounput

# The generated header from flex can be useful
BUILT_SOURCES = xdl_parser.h

SHARED_SRC	= ../bitarray.c ../bitheader.c ../filedump.c \
		../localpips.c  ../wiring.c ../keyfile.c \
		../analysis.c ../connexity.c ../xdlout.c ../sites.c \
		../bitstream_write.c
V2_SRC		= ../bitstream.c ../bitstream_parser.c ../codes/crc-ibm.c

bin_PROGRAMS    = xdl2bit
xdl2bit_SOURCES = xdl2bit.c xdl_lexer.l xdl_parser.y parser.h $(SHARED_SRC) $(V2_SRC)
xdl2bit_CFLAGS	= $(AM_CFLAGS) @GLIB_CFLAGS@ -DVIRTEX2
xdl2bit_LDADD	= @GLIB_LIBS@ @GTK_LIBS@

