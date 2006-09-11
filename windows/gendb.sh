#! /bin/bash
BASEDIR=$1
OUTNAME=$2

pushd $BASEDIR
FILES=`find data/* -type d | sed -e 's:/:\\\\:g' | awk '{ print "File /r ${SRCDIR}\\\\" $0 }'`
popd

echo "$FILES" > ${OUTNAME}
