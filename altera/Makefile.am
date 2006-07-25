AM_CPPFLAGS	= -Wall -Werror -Wunused -O3 -g -DDATADIR=\"$(pkgdatadir)\"

bin_PROGRAMS	= altera
altera_SOURCES	= bitarray.c debit.c bitstream.c
altera_CFLAGS	= @GLIB_CFLAGS@ #-DG_DISABLE_ASSERT -DDEBIT_DEBUG=0
altera_LDFLAGS	= @GLIB_LIBS@

