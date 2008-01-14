#! /bin/bash

. log-functions

COMPARE="cmp -s"
#MAKE is set by make itself from above

function get_dirs() {
    DIRS=`ls -d $1/*/`
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
	${MAKE} -s --no-print-directory -f $MAKEFILE $design.$suffix && \
	    ${COMPARE} $design.$suffix $design.$suffix.golden || \
	    log_failure_msg "COMPRESSED BITSTREAM FAILED";

	${MAKE} -s --no-print-directory -f $MAKEFILE ${design}_u.$suffix && \
	    ${COMPARE} $design.$suffix ${design}_u.$suffix || \
	    log_failure_msg "UNCOMPRESSED BITSTREAM FAILED";

	log_success_msg "PASSED";
    else
        log_warning_msg "NO REFERENCE";
    fi
}

function compare_bitstreams() {
    local bitbase=$1;
    local bitgen=$2;

    ${MAKE} -s --no-print-directory -f $MAKEFILE ${bitgen} && \
    ${COMPARE} ${bitbase} ${bitgen} &> /dev/null

    if test $? -ne 0; then
	log_warning_msg "DIFFERING"
    else
	log_success_msg "PASSED"
    fi
}

function check_write() {
    local design=$1;
    echo -ne "rw, from compressed\t"
    compare_bitstreams ${design}_u.bit ${design}.rewrite

    # This will fail as the header is not the same. How can we manage that properly ?
    echo -ne "rw, from uncompressed\t"
    compare_bitstreams ${design}_u.bit ${design}_u.rewrite
}

function check_xdl_param() {
    local design=$1
    local name=$2

    # then debit it again and check pips against the golden ref
    ${MAKE} -s --no-print-directory -f $MAKEFILE $design.xdl2bit.$name && \
	${COMPARE} $design.xdl2bit.$name $design.$name.golden
    local result=$?
    echo -ne "\t\t\t";
    if test $result -ne 0; then
	log_warning_msg "$name DIFFER"
    else
	log_success_msg "$name OK"
    fi
}

function check_xdl() {
    local design=$1
    echo -ne "xdl2bit\t\t\t"

    ${MAKE} -s --no-print-directory -f $MAKEFILE $design.xdl2bit || \
	log_failure_msg "FAILED"
    log_success_msg "GENERATED"

    check_xdl_param $design "lut"
    check_xdl_param $design "pip"
    check_xdl_param $design "bram"
}

function produce_suffix() {
    local design=$1;
    local suffix=$2;

    echo -ne "$suffix\t\t\t"
    ${MAKE} -s --no-print-directory -f $MAKEFILE $design.$suffix && \
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
    #Test that bitstream rewrite function is somewhat OK
    check_write ${DESIGN_NAME}

    #Test that the xdl2bit tool is okay
    check_xdl ${DESIGN_NAME}

#    ${MAKE} -s --no-print-directory CLEANDIR=$designs -f $MAKEFILE clean
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
#CALLED_FUN=force_design
