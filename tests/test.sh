#! /bin/bash

MAKEFILE=$srcdir/testmake.mk

function get_dirs() {
    DIRS=`find $1 -mindepth 1 -maxdepth 1 -type d`
}

function test_all() {
    get_dirs $srcdir
    for family in $DIRS; do
	echo "Descending in family $family"
	test_family || exit 1
    done
}

function test_family() {
    get_dirs $family
    for chip in $DIRS; do
	echo "Descending in chip $chip"
	test_chip || exit 1
    done
}

function test_chip() {
    get_dirs $chip
    for designs in $DIRS; do
	echo "Descending in designs $design"
	test_designs || exit 1
    done
}

function test_designs {
    for design in `ls $designs/*.xdl`; do
	test_design ${design%.xdl} || exit 1;
	make CLEANDIR=$designs/ -f $MAKEFILE clean
    done
}

function check_suffix() {
    local design=$1;
    local suffix=$2;

    echo "Testing $suffix"
    make -f $MAKEFILE $design.$suffix && \
    make -f $MAKEFILE ${design}_u.$suffix && \
    diff -q $design.$suffix ${design}_u.$suffix || exit 1
    diff $design.$suffix $design.$suffix.golden || exit 1
}

function test_design() {
    local DESIGN_NAME=$1;
    echo "Testing $DESIGN_NAME"

    #Test that the frames are identical for uncompressed and compressed bitstreams
    make -f $MAKEFILE ${DESIGN_NAME}.frames && \
    make -f $MAKEFILE ${DESIGN_NAME}_u.frames && \
    diff -q ${DESIGN_NAME}.frames ${DESIGN_NAME}_u.frames || exit 1

    #Test that the debit output is similar for uncompressed and compressed bitstreams
    check_suffix ${DESIGN_NAME} bram
    check_suffix ${DESIGN_NAME} lut
    check_suffix ${DESIGN_NAME} pip
}

test_all
