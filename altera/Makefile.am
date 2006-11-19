AM_CPPFLAGS	= -I$(top_srcdir) -DDATADIR=\"$(pkgdatadir)\"
AM_CFLAGS	= -Wall -Werror -Wunused -O3 -g

bin_PROGRAMS	= altera
altera_SOURCES	= bitarray.c debit.c bitstream.c bitstream_header.c \
		  crc-ccitt.c crc-ccitt.h \
		  bitarray.h  bitstream.h bitstream_header.h
altera_CFLAGS	= $(AM_CFLAGS) @GLIB_CFLAGS@ #-DG_DISABLE_ASSERT -DDEBIT_DEBUG=0
altera_LDADD	= @GLIB_LIBS@

