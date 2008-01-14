AM_CPPFLAGS	= -DDATADIR=\"$(pkgdatadir)\" -I$(top_srcdir)
AM_CFLAGS	= -O3

AM_YFLAGS = -d
AM_LFLAGS = -Cfe --perf-report --verbose --nounput

# The generated header from flex can be useful
BUILT_SOURCES = xdl_parser.h

SHARED_SRC	= ../bitarray.c ../bitheader.c ../filedump.c \
		../localpips.c  ../wiring.c ../keyfile.c \
		../analysis.c ../connexity.c ../xdlout.c ../sites.c \
		../bitstream_write.c
PARSER_SRC	= xdl2bit.c xdl_lexer.l xdl_parser.y parser.h

bin_PROGRAMS    = xdl2bit xdl2bit_s3 xdl2bit_v4 xdl2bit_v5

V2_S3_SRC	= ../bitstream.c ../bitstream_parser.c ../codes/crc-ibm.c
xdl2bit_SOURCES = $(PARSER_SRC) $(SHARED_SRC) $(V2_S3_SRC)
xdl2bit_CFLAGS	= $(AM_CFLAGS) @GLIB_CFLAGS@ -DVIRTEX2
xdl2bit_LDADD	= @GLIB_LIBS@ @GTK_LIBS@

xdl2bit_s3_SOURCES = $(PARSER_SRC) $(SHARED_SRC) $(V2_S3_SRC)
xdl2bit_s3_CFLAGS  = $(AM_CFLAGS) @GLIB_CFLAGS@ -DSPARTAN3
xdl2bit_s3_LDADD   = @GLIB_LIBS@ @GTK_LIBS@

V4_V5_SRC	   = ../bitstream_v4.c ../bitstream_parser_common.c ../codes/crc32-c.c ../codes/xhamming.c
xdl2bit_v4_SOURCES = $(PARSER_SRC) $(SHARED_SRC) $(V4_V5_SRC)
xdl2bit_v4_CFLAGS  = $(AM_CFLAGS) @GLIB_CFLAGS@ -DVIRTEX4
xdl2bit_v4_LDADD   = @GLIB_LIBS@ @GTK_LIBS@

xdl2bit_v5_SOURCES = $(PARSER_SRC) $(SHARED_SRC) $(V4_V5_SRC)
xdl2bit_v5_CFLAGS  = $(AM_CFLAGS) @GLIB_CFLAGS@ -DVIRTEX5
xdl2bit_v5_LDADD   = @GLIB_LIBS@ @GTK_LIBS@
