#! /bin/bash

#This script converts windows .pc files for gtk to a linux-xcompile-environment safe version

. cross.env

PCDIR=$WINGTK_PATH/lib/pkgconfig

echo "Converting .pc files in $PCDIR"

cd $PCDIR

for f in *.pc; do
   if grep 'prefix=' $f >/dev/null 2>&1 ; then
	cat $f | sed s+^prefix=.*$+prefix=$WINGTK_PATH+ > $f.tmp
        mv $f.tmp $f
  fi
done
