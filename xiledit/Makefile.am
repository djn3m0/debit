AM_CPPFLAGS	= -DDATADIR=\"$(pkgdatadir)\" -I$(top_srcdir)
AM_CFLAGS	= -Wall -Werror -Wunused -O3 -g

SHARED_SRC	= ../bitarray.c ../bitheader.c ../bitstream_parser.c ../filedump.c \
		../localpips.c ../bitstream.c ../wiring.c ../keyfile.c \
		../analysis.c ../connexity.c
#SHARED_COUCOU	= $(addprefix ../,$(SHARED_SRC))

bin_PROGRAMS	= xiledit
xiledit_SOURCES	= xiledit.c xildraw.c xildraw.h \
		  ../sites.c ../sites_draw.c ../wiring_draw.c $(SHARED_SRC)
xiledit_CFLAGS	= $(AM_CFLAGS) @GLIB_CFLAGS@ @CAIRO_CFLAGS@ @GTK_CFLAGS@ @LIBGLADE_CFLAGS@
xiledit_LDFLAGS	= @GLIB_LIBS@ @CAIRO_LIBS@ @GTK_LIBS@ @LIBGLADE_LIBS@

nobase_dist_pkgdata_DATA = xiledit.glade
dist_noinst_DATA = xiledit.gladep
