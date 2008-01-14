SUBDIRS		= correlation

AM_CPPFLAGS	= -O3

bin_PROGRAMS		= diff_bin_pos
diff_bin_pos_SOURCES	= diff_bin.c
diff_bin_pos_CFLAGS	= @GLIB_CFLAGS@ #-DG_DISABLE_ASSERT -DDEBIT_DEBUG=0
diff_bin_pos_LDADD	= @GLIB_LIBS@

