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

dnl Checks for libraries
AC_CHECK_LIB([m], [sqrt])

dnl Check for glib
GLIB_REQUIRED=2.2.0
PKG_CHECK_MODULES(GLIB, [glib-2.0 >= $GLIB_REQUIRED])
AC_SUBST(GLIB_CFLAGS)
AC_SUBST(GLIB_LIBS)

dnl ===========================================================================
dnl
dnl Define a macro to enable feature
dnl  - Macro: DEBIT_FEATURE_ENABLE (ID, NAME, DEFAULT, COMMANDS-TO-CHECK-IT)
dnl
dnl Where COMMANDS-TO-CHECK-IT should set $use_ID to something other than yes if the
dnl backend cannot be built.
dnl
dnl Copied verbatim from CAIRO_BACKEND_ENABLE
dnl
AC_DEFUN([DEBIT_FEATURE_ENABLE],
         [AC_ARG_ENABLE([$1],
                         AS_HELP_STRING([--enable-$1=@<:@no/auto/yes@:>@],
                                        [Enable debit's $2 feature @<:@default=$3@:>@]),
                         use_$1=$enableval, use_$1=$3)
	  if test "x$use_$1" = xno; then
	    use_$1="no (disabled, use --enable-$1 to enable)"
	  else
            AC_CACHE_CHECK([for debit's $2 feature], debit_cv_use_$1,
			   [echo
			    saved_use_$1=$use_$1
			    use_$1=yes
			    $4
			    debit_cv_use_$1=$use_$1
			    use_$1=$saved_use_$1
			    AC_MSG_CHECKING([whether debit's $2 feature could be enabled])])
	    case $use_$1 in
	      yes)
	        if test "x$debit_cv_use_$1" = xyes; then
		  use_$1=yes
		else
	          AC_MSG_ERROR([requested feature $2 could not be enabled])
		fi
		;;
	      auto)
	        use_$1=$debit_cv_use_$1
		;;
	      *)
	        AC_MSG_ERROR([invalid argument passed to --enable-$1: $use_$1, should be one of @<:@no/auto/yes@:>@])
		;;
	    esac
	  fi])

dnl ===========================================================================

dnl Check for Cairo pdf/ps backend for bit2pdf
CAIRO_REQUIRED=1.2.0
DEBIT_FEATURE_ENABLE(cairo, Cairo, auto, [
  PKG_CHECK_MODULES(CAIRO, [cairo-pdf >= $CAIRO_REQUIRED cairo-ps >= $CAIRO_REQUIRED],
			   [use_cairo=yes], [use_cairo=no])
])
AM_CONDITIONAL([BUILD_BIT2PDF], [test "x$use_cairo" = xyes])
AC_SUBST(CAIRO_CFLAGS)
AC_SUBST(CAIRO_LIBS)

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
