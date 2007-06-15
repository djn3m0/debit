#! /bin/bash

source $srcdir/test.sh

echo "***********************"
echo "Testing Virtex-5 Family"
echo "***********************"

MAKEFILE="$srcdir/testmake.mk DEBIT=\$(top_builddir)/debit_v5 DUMPARG=--unkdump"
family=virtex5
test_family
