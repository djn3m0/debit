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
