AM_CPPFLAGS	= -DDATADIR=\"$(pkgdatadir)\" -I$(top_srcdir)
AM_CFLAGS	= -Wall -Werror -Wunused -O3 -g

glade_sources   = xiledit.glade xiledit.gladep
glade_generated = interface.c interface.h support.c support.h

DRAWING_SRC	= xiledit.c xildraw.c xildraw.h callbacks.c callbacks.h \
		../sites.c ../sites_draw.c ../wiring_draw.c
SHARED_SRC	= ../bitarray.c ../bitheader.c ../filedump.c \
		../localpips.c  ../wiring.c ../keyfile.c \
		../analysis.c ../connexity.c ../xdlout.c
V2_SRC		= ../bitstream.c ../bitstream_parser.c ../codes/crc-ibm.c
V4_SRC		= ../bitstream_v4.c ../bitstream_parser_common.c ../codes/crc32-c.c

bin_PROGRAMS	=

if BUILD_GUI
  bin_PROGRAMS += xiledit xiledit_v5 xiledit_v4 xiledit_s3
endif

if BUILD_GUI_DIRECTFB
  bin_PROGRAMS += xiledit_direct
endif

xiledit_SOURCES	= $(DRAWING_SRC) $(SHARED_SRC) $(V2_SRC) $(glade_generated)
xiledit_CFLAGS	= $(AM_CFLAGS) @GLIB_CFLAGS@ @GTK_CFLAGS@ -DVIRTEX2
xiledit_LDADD	= @GLIB_LIBS@ @GTK_LIBS@

#This should just be a flag in configure
xiledit_direct_SOURCES	= $(xiledit_SOURCES)
xiledit_direct_CFLAGS	= $(AM_CFLAGS) @GLIB_CFLAGS@ @GTK_DIRECTFB_CFLAGS@ -DVIRTEX2
xiledit_direct_LDADD	= @GLIB_LIBS@ @GTK_DIRECTFB_LIBS@

#Xiledit, Spartan-3 verstion
xiledit_s3_SOURCES	= $(DRAWING_SRC) $(SHARED_SRC) $(V2_SRC) $(glade_generated)
xiledit_s3_CFLAGS	= $(AM_CFLAGS) @GLIB_CFLAGS@ @GTK_CFLAGS@ -DSPARTAN3
xiledit_s3_LDADD	= @GLIB_LIBS@ @GTK_LIBS@

#Xiledit, virtex-5 version
xiledit_v5_SOURCES	= $(DRAWING_SRC) $(SHARED_SRC) $(V4_SRC) $(glade_generated)
xiledit_v5_CFLAGS	= $(AM_CFLAGS) @GLIB_CFLAGS@ @GTK_CFLAGS@ -DVIRTEX5
xiledit_v5_LDADD	= $(xiledit_LDADD)

#Xiledit, virtex-4 version
xiledit_v4_SOURCES	= $(DRAWING_SRC) $(SHARED_SRC) $(V4_SRC) $(glade_generated)
xiledit_v4_CFLAGS	= $(AM_CFLAGS) @GLIB_CFLAGS@ @GTK_CFLAGS@ -DVIRTEX4
xiledit_v4_LDADD	= $(xiledit_LDADD)

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
