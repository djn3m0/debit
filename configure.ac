#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_INIT([debit], [0.1], [jean-baptiste.note@m4x.org])
AC_CONFIG_SRCDIR([debit.c])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([-Wall dist-bzip2])

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

dnl Check for Cairo pdf/ps backend
CAIRO_REQUIRED=1.2.0
PKG_CHECK_MODULES(CAIRO, [cairo-pdf >= $CAIRO_REQUIRED cairo-ps >= $CAIRO_REQUIRED],
			 [want_bit2pdf=yes], [want_bit2pdf=no])
AC_SUBST(CAIRO_CFLAGS)
AC_SUBST(CAIRO_LIBS)

AM_CONDITIONAL([BUILD_BIT2PDF], [test "x$want_bit2pdf" = xyes])
if test "x$want_bit2pdf" = "xno"; then
  AC_MSG_WARN(Not building bit2pdf tool)
fi

dnl First version which supports cairo ?
GTK_REQUIRED=2.8.0
PKG_CHECK_MODULES(GTK, [gtk+-2.0 >= $GTK_REQUIRED],
		       [want_gui=yes], [want_gui=no])
AC_SUBST(GTK_CFLAGS)
AC_SUBST(GTK_LIBS)

AM_CONDITIONAL(BUILD_GUI, [test "x$want_gui" = xyes])
if test "x$want_gui" = "xno"; then
  AC_MSG_WARN(Not building gui tools)
fi

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
