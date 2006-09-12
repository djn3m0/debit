AM_CPPFLAGS	= -Wall -Werror -Wunused -O3 -g -DDATADIR=\"$(pkgdatadir)\"

bin_PROGRAMS	= xiledit
xiledit_SOURCES	= xiledit.c xildraw.c xildraw.h \
		  ../sites.c ../keyfile.c ../sites_draw.c ../wiring_draw.c
nobase_dist_pkgdata_DATA = xiledit.glade
nodist_data_DATA = xiledit.gladep
xiledit_CFLAGS	= @GLIB_CFLAGS@ @CAIRO_CFLAGS@ @GTK_CFLAGS@ @LIBGLADE_CFLAGS@
xiledit_LDFLAGS	= @GLIB_LIBS@ @CAIRO_LIBS@ @GTK_LIBS@ @LIBGLADE_LIBS@
