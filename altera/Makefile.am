AM_CPPFLAGS	= -Wall -Werror -Wunused -O3 -g -DDATADIR=\"$(pkgdatadir)\"

bin_PROGRAMS	= altera
altera_SOURCES	= bitarray.c debit.c bitstream.c bitstream_header.c \
		  bitarray.h  bitstream.h bitstream_header.h
altera_CFLAGS	= @GLIB_CFLAGS@ #-DG_DISABLE_ASSERT -DDEBIT_DEBUG=0
altera_LDFLAGS	= @GLIB_LIBS@

