#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_INIT([debit], [0.1], [jean-baptiste.note@m4x.org])
AC_CONFIG_SRCDIR([debit.c])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([1.9 -Wall dist-bzip2])

dnl Checks for programs
AC_PROG_CC
AC_PROG_YACC
AC_PROG_LEX
AC_PROG_INSTALL

dnl System-specific setup
AX_CHECK_ALIGNED_ACCESS_REQUIRED

dnl Checks for libraries
AC_CHECK_LIB([m], [sqrt])

dnl Check for glib
GLIB_REQUIRED=2.2.0
PKG_CHECK_MODULES(GLIB, [glib-2.0 >= $GLIB_REQUIRED])
AC_SUBST(GLIB_CFLAGS)
AC_SUBST(GLIB_LIBS)

dnl Check for Cairo pdf/ps backend for bit2pdf
CAIRO_REQUIRED=1.2.0
DEBIT_FEATURE_ENABLE(cairo, Cairo support, auto, [
  PKG_CHECK_MODULES(CAIRO, [cairo >= $CAIRO_REQUIRED],
			   [use_cairo=yes], [use_cairo=no])
])
AM_CONDITIONAL([BUILD_BIT2PDF], [test "x$use_cairo" = xyes])
AC_SUBST(CAIRO_CFLAGS)
AC_SUBST(CAIRO_LIBS)

DEBIT_FEATURE_ENABLE(pdfcairo, Cairo pdf support, auto, [
  PKG_CHECK_MODULES(CAIRO_PDF, [cairo-pdf >= $CAIRO_REQUIRED],
			       [use_pdfcairo=yes], [use_pdfcairo=no])
])
if test "x$use_pdfcairo" = xyes; then
   CPPFLAGS="$CPPFLAGS -DPDF_CAIRO "
fi
AC_SUBST(CAIRO_PDF_CFLAGS)
AC_SUBST(CAIRO_PDF_LIBS)

DEBIT_FEATURE_ENABLE(pscairo, Cairo postscript support, auto, [
  PKG_CHECK_MODULES(CAIRO_PS, [cairo-ps >= $CAIRO_REQUIRED],
			      [use_pscairo=yes], [use_pscairo=no])
])
if test "x$use_pscairo" = xyes; then
   CPPFLAGS="$CPPFLAGS -DPS_CAIRO "
fi
AC_SUBST(CAIRO_PS_CFLAGS)
AC_SUBST(CAIRO_PS_LIBS)

DEBIT_FEATURE_ENABLE(svgcairo, Cairo svg support, auto, [
  PKG_CHECK_MODULES(CAIRO_SVG, [cairo-svg >= $CAIRO_REQUIRED],
			       [use_svgcairo=yes], [use_svgcairo=no])
])
if test "x$use_svgcairo" = xyes; then
   CPPFLAGS="$CPPFLAGS -DSVG_CAIRO "
fi
AC_SUBST(CAIRO_SVG_CFLAGS)
AC_SUBST(CAIRO_SVG_LIBS)

dnl Check for gtk+ for building the gui
GTK_REQUIRED=2.8.0
DEBIT_FEATURE_ENABLE(gtk, GTK+, auto, [
  PKG_CHECK_MODULES(GTK, [gtk+-2.0 >= $GTK_REQUIRED],
			 [use_gtk=yes], [use_gtk=no])
])
AM_CONDITIONAL([BUILD_GUI], [test "x$use_gtk" = xyes])
AC_SUBST(GTK_CFLAGS)
AC_SUBST(GTK_LIBS)

dnl Check for gtk+directfb for building the directfb version of the gui
DEBIT_FEATURE_ENABLE(directfb, DirectFB, no, [
  PKG_CHECK_MODULES(GTK_DIRECTFB, [gtk+-directfb-2.0 >= $GTK_REQUIRED],
				  [use_directfb=yes], [use_directfb=no])
])
AM_CONDITIONAL([BUILD_GUI_DIRECTFB], [test "x$use_directfb" = xyes])
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
		data/Makefile \
		data/spartan3/Makefile \
		data/virtex2/Makefile \
		data/virtex4/Makefile \
		data/virtex5/Makefile \
		xdl/Makefile \
		debian/Makefile rpm/Makefile \
		windows/Makefile windows/scripts/Makefile \
		altera/Makefile	\
		codes/Makefile utils/Makefile utils/correlation/Makefile \
		xiledit/Makefile \
		desktop/Makefile desktop/artwork/Makefile \
		doc/Makefile doc/man/Makefile])
AC_OUTPUT
