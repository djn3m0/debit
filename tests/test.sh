#! /bin/bash

. log-functions

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
	${CALLED_FUN} ${design%.xdl} || exit 1;
    done
}

function check_suffix() {
    local design=$1;
    local suffix=$2;

    echo -ne "$suffix\t\t\t"

    if [ -e $design.$suffix.golden ]; then
	make -s --no-print-directory -f $MAKEFILE $design.$suffix && \
	    diff -q $design.$suffix $design.$suffix.golden || \
	    log_failure_msg "COMPRESSED BITSTREAM FAILED";

	make -s --no-print-directory -f $MAKEFILE ${design}_u.$suffix && \
	    diff -q $design.$suffix ${design}_u.$suffix || \
	    log_failure_msg "UNCOMPRESSED BITSTREAM FAILED";

	log_success_msg "PASSED";
    else
        log_warning_msg "NO REFERENCE";
    fi
}

function produce_suffix() {
    local design=$1;
    local suffix=$2;

    echo -ne "$suffix\t\t\t"
    make -s --no-print-directory -f $MAKEFILE $design.$suffix && \
    mv -f $design.$suffix $design.$suffix.golden || \
	log_failure_msg "GENERATION FAILED"

    log_warning_msg "GENERATED";
}

function test_design() {
    local DESIGN_NAME=$1;
    echo -ne "Testing design\t\t";
    log_fancy_msg "$DESIGN_NAME";

    #Test that the debit output is similar for uncompressed and compressed bitstreams
    check_suffix ${DESIGN_NAME} frames
    check_suffix ${DESIGN_NAME} bram
    check_suffix ${DESIGN_NAME} lut
    check_suffix ${DESIGN_NAME} pip

    make -s --no-print-directory CLEANDIR=$designs -f $MAKEFILE clean
}

function force_design() {
    local DESIGN_NAME=$1;
    echo -ne "Generating for design\t";
    log_fancy_msg "$DESIGN_NAME";

    produce_suffix ${DESIGN_NAME} frames
    produce_suffix ${DESIGN_NAME} bram
    produce_suffix ${DESIGN_NAME} lut
    produce_suffix ${DESIGN_NAME} pip
}

CALLED_FUN=test_design
