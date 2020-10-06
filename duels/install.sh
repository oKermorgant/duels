#!/bin/bash

PREFIX=/opt
DIR=`dirname "$0"`

if [ $# -eq 1 ]
  then
    PREFIX=$1
fi

# prep debian package
DIR=`(realpath $DIR)`
rm -rf "$DIR/../debian"
mkdir -p "$DIR/../debian"
echo $PREFIX >> $DIR/../debian/.path

PREFIX=$PREFIX/duels

echo "$DIR -> $PREFIX"

mkdir -m 755 -p $PREFIX/bin
rsync -av --chmod=755 $DIR/include $PREFIX/
rsync -av --chmod=755 $DIR/*.py $PREFIX/
rsync -av --chmod=755 $DIR/templates $PREFIX/


