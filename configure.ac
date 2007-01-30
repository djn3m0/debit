#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_INIT([debit], [0.1], [jean-baptiste.note@m4x.org])
AC_CONFIG_SRCDIR([debit.c])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([1.9 -Wall dist-bzip2])

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

dnl Check for Cairo pdf/ps backend for bit2pdf
CAIRO_REQUIRED=1.2.0
AC_ARG_WITH(cairo, [  --with-cairo	allow use of cairo [default=yes]], with_cairo=$withval, with_cairo=yes)
if test "x$with_cairo" = "xyes"; then
  PKG_CHECK_MODULES(CAIRO, [cairo-pdf >= $CAIRO_REQUIRED cairo-ps >= $CAIRO_REQUIRED])
fi
AM_CONDITIONAL([BUILD_BIT2PDF], [test "x$with_cairo" = xyes])
AC_SUBST(CAIRO_CFLAGS)
AC_SUBST(CAIRO_LIBS)

dnl Check for gtk+ for building the gui
GTK_REQUIRED=2.8.0
AC_ARG_WITH(gtk, [  --with-gtk	allow use of gtk [default=yes]], with_gtk=$withval, with_gtk=yes)
if test "x$with_gtk" = "xyes"; then
  PKG_CHECK_MODULES(GTK, [gtk+-2.0 >= $GTK_REQUIRED])
fi
AM_CONDITIONAL([BUILD_GUI], [test "x$with_gtk" = xyes])
AC_SUBST(GTK_CFLAGS)
AC_SUBST(GTK_LIBS)

dnl Check for gtk+directfb for building the directfb version of the gui
AC_ARG_WITH(directfb, [  --with-directfb	allow use of directfb [default=no]], with_directfb=$withval, with_directfb=no)
if test "x$with_directfb" = "xyes"; then
  PKG_CHECK_MODULES(GTK_DIRECTFB, [gtk+-directfb-2.0 >= $GTK_REQUIRED])
fi
AM_CONDITIONAL([BUILD_GUI_DIRECTFB], [test "x$with_directfb" = xyes])
AC_SUBST(GTK_DIRECTFB_CFLAGS)
AC_SUBST(GTK_DIRECTFB_LIBS)

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

# Enable non-debug mode
AC_ARG_ENABLE(asserts, [  --disable-asserts    remove assertions in the code], enable_asserts=$enableval, enable_asserts=yes)
if test "x$enable_asserts" = "xno"; then
   CPPFLAGS="$CPPFLAGS -DG_DISABLE_CHECKS -DG_DISABLE_ASSERT "
fi

AC_ARG_ENABLE(debug, [  --disable-debug    disable debug print functions], enable_debug=$enableval, enable_debug=yes)
if test "x$enable_debug" = "xno"; then
   CPPFLAGS="$CPPFLAGS -DDEBIT_DEBUG=0 "
fi

AC_ARG_ENABLE(wiredb, [  --enable-wiredb    enable built-in wire database], enable_wiredb=$enableval, enable_wiredb=no)
if test "x$enable_wiredb" = "xyes"; then
   CPPFLAGS="$CPPFLAGS -D__COMPILED_WIREDB "
fi

AC_ARG_ENABLE(pipsdb, [  --enable-pipsdb    enable built-in pips database], enable_pipsdb=$enableval, enable_pipsdb=no)
if test "x$enable_pipsdb" = "xyes"; then
   CPPFLAGS="$CPPFLAGS -D__COMPILED_PIPSDB "
fi

AC_CONFIG_FILES([Makefile tests/Makefile \
		data/Makefile data/virtex2/Makefile data/virtex4/Makefile \
		debian/Makefile rpm/Makefile windows/Makefile \
		altera/Makefile \
		codes/Makefile utils/Makefile utils/correlation/Makefile \
		xiledit/Makefile \
		desktop/Makefile desktop/artwork/Makefile
		doc/Makefile doc/man/Makefile])
AC_OUTPUT
