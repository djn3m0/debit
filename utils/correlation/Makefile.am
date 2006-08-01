AM_CPPFLAGS	= -Wall -Werror -Wunused -O3 -g

bin_PROGRAMS		= bitisolation
bitisolation_SOURCES	= state_db.c pip_db.c bitisolation.c \
			$(top_srcdir)/altera/bitarray.c algo_basic.c \
			algos.h  bitisolation_db.h  bitisolation.h  pip_db.h

bitisolation_CFLAGS	= @GLIB_CFLAGS@
bitisolation_LDFLAGS	= @GLIB_LIBS@

