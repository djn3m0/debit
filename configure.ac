#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_INIT([debit], [0.1], [jean-baptiste.note@m4x.org])
AC_CONFIG_SRCDIR([debit.c])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([debit],[0.1])

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

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
