#! /bin/bash
BASEDIR=$1
OUTNAME=$2

pushd $BASEDIR
DIRS=`find data/* -type d | sed -e 's:/:\\\\:g' | awk '{ print "File /r ${SRCDIR}\\\\" $0 }'`
#Then top-level files ending in .db
FILES=`find data -maxdepth 1 -name '*.db' | sed -e 's:/:\\\\:g' | awk '{ print "File ${SRCDIR}\\\\" $0 }'`
popd

echo "$DIRS" > ${OUTNAME}
echo "$FILES" >> ${OUTNAME}
