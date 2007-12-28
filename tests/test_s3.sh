#! /bin/bash

source $srcdir/test.sh

echo "************************"
echo "Testing Spartan-3 Family"
echo "************************"

MAKEFILE="$srcdir/testmake.mk DEBIT=\$(top_builddir)/debit_s3 DUMPARG=--framedump XDL2BIT=\$(top_builddir)/xdl/xdl2bit_s3"
family=spartan3
test_family
