#! /bin/bash

source $srcdir/test.sh

echo "************************"
echo "Testing Virtex-II Family"
echo "************************"

MAKEFILE="$srcdir/testmake.mk DEBIT=\$(top_builddir)/debit DUMPARG=--framedump"
family=virtex2
test_family
