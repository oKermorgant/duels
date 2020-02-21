#!/bin/bash

PREFIX=/opt
DIR=`dirname "$0"`

if [ $# -eq 1 ]
  then
    PREFIX=$1
fi
PREFIX=$PREFIX/duels

echo "$DIR -> $PREFIX"

mkdir -m 755 -p $PREFIX/bin
rsync -av --chmod=755 $DIR/include $PREFIX/
rsync -av --chmod=755 $DIR/*.py $PREFIX/
rsync -av --chmod=755 $DIR/templates $PREFIX/
