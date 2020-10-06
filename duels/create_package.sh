#!/usr/bin/bash
DST=`dirname "$0"`
DST="$DST/../debian/"

SRC=`cat "$DST/.path"`
echo "Creating package from $SRC"

mkdir -p "$DST/duels/DEBIAN"
mkdir -p "$DST/duels/$SRC"
cp -r $SRC/duels $DST/duels/$SRC

SIZE=$(du -s --block-size=1024 $DST/duels/$SRC | cut -f1)

echo "Package: duels
Version: 1.0
Section: custom
Priority: optional
Architecture: all
Essential: no
Installed-Size: $SIZE
Depends: python3-pygame, libzmq3-dev
Maintainer: olivier.kermorgant@ec-nantes.fr
Description: The Duels package to practice game AI's" > "$DST/duels/DEBIAN/control"

cd $DST
dpkg-deb --build duels
