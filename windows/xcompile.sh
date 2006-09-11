#! /bin/bash

#small script to cross-compile the packages for windows.

BUILD_DIR=build
SOURCE_DIR=../..

. cross.env

#get the absolute path
SOURCE_DIR=$(cd $(dirname ${SOURCE_DIR}) && pwd)

echo "Compiling for target ${TARGET} in ${BUILD_DIR} from ${SOURCE_DIR}"
echo "PKGCONFIG path is ${PKG_CONFIG_PATH}"

mkdir $BUILD_DIR || exit 1

pushd $BUILD_DIR
$SOURCE_DIR/configure --target=${TARGET} --host=${TARGET} --prefix=${PREFIX} && \
make && \
make -C windows/ installer
popd

#get back the resulting installer, this is what we want after all
cp $BUILD_DIR/windows/debit-*.exe .

rm -Rf $BUILD_DIR
