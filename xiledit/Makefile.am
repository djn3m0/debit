AM_CPPFLAGS	= -DDATADIR=\"$(pkgdatadir)\" -I$(top_srcdir)
AM_CFLAGS	= -Wall -Werror -Wunused -O3 -g

glade_sources   = xiledit.glade xiledit.gladep
glade_generated = interface.c interface.h support.c support.h

SHARED_SRC	= ../bitarray.c ../bitheader.c ../bitstream_parser.c ../filedump.c \
		../localpips.c ../bitstream.c ../wiring.c ../keyfile.c \
		../analysis.c ../connexity.c ../codes/crc-ibm.c
#SHARED_COUCOU	= $(addprefix ../,$(SHARED_SRC))

bin_PROGRAMS	=

if BUILD_GUI
  bin_PROGRAMS += xiledit
endif

if BUILD_GUI_DIRECTFB
  bin_PROGRAMS += xiledit_direct
endif

xiledit_SOURCES	= xiledit.c xildraw.c xildraw.h callbacks.c callbacks.h \
		  ../sites.c ../sites_draw.c ../wiring_draw.c $(SHARED_SRC) $(glade_generated)
xiledit_CFLAGS	= $(AM_CFLAGS) -DVIRTEX2 @GLIB_CFLAGS@ @GTK_CFLAGS@
xiledit_LDADD	= @GLIB_LIBS@ @GTK_LIBS@

xiledit_direct_SOURCES	= $(xiledit_SOURCES)
xiledit_direct_CFLAGS	= $(AM_CFLAGS) -DVIRTEX2 @GLIB_CFLAGS@ @GTK_DIRECTFB_CFLAGS@
xiledit_direct_LDADD	= @GLIB_LIBS@ @GTK_DIRECTFB_LIBS@

BUILT_SOURCES	= $(glade_generated)
EXTRA_DIST = $(glade_sources)

$(glade_generated): $(glade_sources)
	xvfb-run glade-2 -w $<

MAINTAINERCLEANFILES = $(glade_generated)

# My own unpretty targets

# Cairo profiling
CALLGRIND_ARGS=--instr-atstart=no --toggle-collect=draw_all_wires --dump-after=draw_all_wires
profile-exec: $(bin_PROGRAMS)
	export LD_LIBRARY_PATH=/usr/lib/debug && \
	callgrind --dump-instr=yes $(top_builddir)/xiledit/$< -i ~/s3c2000.bit
