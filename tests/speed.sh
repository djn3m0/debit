#! /bin/bash

source $srcdir/test.sh

echo "************************"
echo "Speed Virtex-II Family"
echo "************************"

function speed_design() {
    local DESIGN_NAME=$1;
    echo "Doing speed analysis for design $DESIGN_NAME"
    make -s --no-print-directory -f $MAKEFILE ${DESIGN_NAME}.allspeed
}

CALLED_FUN=speed_design

MAKEFILE="$srcdir/testmake.mk DEBIT=\$(top_builddir)/debit DUMPARG=--framedump"
family=virtex2
test_family

#Gather data in one file
find $builddir -name *.allspeed | xargs cat >> perfs.txt

