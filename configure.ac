#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_INIT([debit], [0.1], [jean-baptiste.note@m4x.org])
AC_CONFIG_SRCDIR([debit.c])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([dist-bzip2])

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL

# Checks for libraries.
AC_CHECK_LIB([m], [sqrt])

# Check for glib
GLIB_REQUIRED=2.2.0
PKG_CHECK_MODULES(GLIB, [glib-2.0 >= $GLIB_REQUIRED])
AC_SUBST(GLIB_CFLAGS)
AC_SUBST(GLIB_LIBS)

dnl Check for Cairo
CAIRO_REQUIRED=1.2.0
PKG_CHECK_MODULES(CAIRO, [cairo >= $CAIRO_REQUIRED])
AC_SUBST(CAIRO_CFLAGS)
AC_SUBST(CAIRO_LIBS)

dnl First version which supports cairo ?
GTK_REQUIRED=2.8.0
PKG_CHECK_MODULES(GTK, [gtk+-2.0 >= $GTK_REQUIRED])
AC_SUBST(GTK_CFLAGS)
AC_SUBST(GTK_LIBS)

dnl completely random requirement
LIBGLADE_REQUIRED=2.4.0
PKG_CHECK_MODULES(LIBGLADE, [libglade-2.0 >= $LIBGLADE_REQUIRED])
AC_SUBST(LIBGLADE_CFLAGS)
AC_SUBST(LIBGLADE_LIBS)

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([fcntl.h inttypes.h limits.h stdlib.h string.h sys/ioctl.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_TYPE_OFF_T
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MEMCMP
AC_FUNC_MMAP
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([memset munmap regcomp strdup strerror strtoul])

AC_CONFIG_FILES([Makefile data/Makefile \
		debian/Makefile rpm/Makefile windows/Makefile \
		altera/Makefile \
		utils/Makefile utils/correlation/Makefile \
		xiledit/Makefile \
		desktop/Makefile desktop/artwork/Makefile])
AC_OUTPUT
