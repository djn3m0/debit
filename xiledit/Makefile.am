AM_CPPFLAGS	= -DDATADIR=\"$(pkgdatadir)\" -I$(top_srcdir)
AM_CFLAGS	= -Wall -Werror -Wunused -O3 -g

glade_sources   = xiledit.glade xiledit.gladep
glade_generated = callbacks.c interface.c support.c callbacks.h interface.h support.h

SHARED_SRC	= ../bitarray.c ../bitheader.c ../bitstream_parser.c ../filedump.c \
		../localpips.c ../bitstream.c ../wiring.c ../keyfile.c \
		../analysis.c ../connexity.c
#SHARED_COUCOU	= $(addprefix ../,$(SHARED_SRC))

bin_PROGRAMS	= xiledit
xiledit_SOURCES	= xiledit.c xildraw.c xildraw.h \
		  ../sites.c ../sites_draw.c ../wiring_draw.c $(SHARED_SRC) $(glade_generated)
BUILT_SOURCES	= $(glade_generated)
xiledit_CFLAGS	= $(AM_CFLAGS) @GLIB_CFLAGS@ @CAIRO_CFLAGS@ @GTK_CFLAGS@
xiledit_LDADD	= @GLIB_LIBS@ @CAIRO_LIBS@ @GTK_LIBS@

EXTRA_DIST = $(glade_sources)

$(glade_sources): $(glade_sources)
	glade-2 -w $<

MAINTAINERCLEANFILES = $(glade_generated)
