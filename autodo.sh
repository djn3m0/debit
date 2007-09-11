#! /bin/bash

aclocal -I ac_macros/
autoconf
autoheader
automake --add-missing
