#! /bin/bash

function get_dirs() {
    DIRS=`find $1 -mindepth 1 -maxdepth 1 -type d`
}

function test_family() {
    get_dirs $family
    for chip in $DIRS; do
	#echo "Descending in chip $chip"
	test_chip || exit 1
    done
}

function test_chip() {
    get_dirs $chip
    for designs in $DIRS; do
	#echo "Descending in designs $designs"
	test_designs || exit 1
    done
}

function test_designs {
    for design in `ls $designs/*.xdl`; do
	test_design ${design%.xdl} || exit 1;
	make -s --no-print-directory CLEANDIR=$designs -f $MAKEFILE clean
    done
}

function check_suffix() {
    local design=$1;
    local suffix=$2;

    echo -n " $suffix"

    if [ -e $design.$suffix.golden ]; then
	make -s --no-print-directory -f $MAKEFILE $design.$suffix && \
	make -s --no-print-directory -f $MAKEFILE ${design}_u.$suffix && \
	diff -q $design.$suffix ${design}_u.$suffix || exit 1
	diff -q $design.$suffix $design.$suffix.golden || exit 1
    else
	echo -n " (no ref)"
    fi
}

function test_design() {
    local DESIGN_NAME=$1;
    echo "Testing design $DESIGN_NAME"

    echo "Checking uncompressed/compressed frame consistency"
    #Test that the frames are identical for uncompressed and compressed bitstreams
    make -s --no-print-directory -f $MAKEFILE ${DESIGN_NAME}.frames && \
    make -s --no-print-directory -f $MAKEFILE ${DESIGN_NAME}_u.frames && \
    diff -q ${DESIGN_NAME}.frames ${DESIGN_NAME}_u.frames || exit 1

    echo -n "Checking"
    #Test that the debit output is similar for uncompressed and compressed bitstreams
    check_suffix ${DESIGN_NAME} bram
    check_suffix ${DESIGN_NAME} lut
    check_suffix ${DESIGN_NAME} pip
    echo "."
}
