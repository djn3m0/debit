#! /bin/bash

source $srcdir/test.sh

echo "***********************"
echo "Testing Virtex-4 Family"
echo "***********************"

MAKEFILE="$srcdir/testmake.mk DEBIT=\$(top_builddir)/debit_v4"
family=virtex4
test_family
