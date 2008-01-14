AM_CPPFLAGS	= -I$(top_srcdir)
AM_CFLAGS	= -O3

bin_PROGRAMS		= bitisolation
bitisolation_SOURCES	= state_db.c pip_db.c bitisolation.c \
			$(top_srcdir)/altera/bitarray.c algo_basic.c \
			algos.h  bitisolation_db.h  bitisolation.h  pip_db.h

bitisolation_CFLAGS	= $(AM_CFLAGS) @GLIB_CFLAGS@
bitisolation_LDADD	= @GLIB_LIBS@

