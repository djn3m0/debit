#! /bin/bash

source $srcdir/test.sh

echo "***********************"
echo "Testing Virtex-5 Family"
echo "***********************"

MAKEFILE="$srcdir/testmake.mk DEBIT=\$(top_builddir)/debit_v5 DUMPARG=--unkdump XDL2BIT=\$(top_builddir)/xdl/xdl2bit_v5"
family=virtex5
test_family
