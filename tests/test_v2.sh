#! /bin/bash

source $srcdir/test.sh

echo "************************"
echo "Testing Virtex-II Family"
echo "************************"

MAKEFILE=$srcdir/testmake.mk
family=virtex2
test_family
