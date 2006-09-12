AM_CPPFLAGS	= -Wall -Werror -Wunused -O3 -g -DDATADIR=\"$(pkgdatadir)\"

SHARED_SRC	= ../bitarray.c ../bitheader.c ../bitstream_parser.c ../filedump.c \
		../localpips.c ../bitstream.c ../wiring.c ../keyfile.c
#SHARED_COUCOU	= $(addprefix ../,$(SHARED_SRC))

bin_PROGRAMS	= xiledit
xiledit_SOURCES	= xiledit.c xildraw.c xildraw.h \
		  ../sites.c ../sites_draw.c ../wiring_draw.c $(SHARED_SRC)
nobase_dist_pkgdata_DATA = xiledit.glade
nodist_data_DATA = xiledit.gladep
xiledit_CFLAGS	= @GLIB_CFLAGS@ @CAIRO_CFLAGS@ @GTK_CFLAGS@ @LIBGLADE_CFLAGS@
xiledit_LDFLAGS	= @GLIB_LIBS@ @CAIRO_LIBS@ @GTK_LIBS@ @LIBGLADE_LIBS@
