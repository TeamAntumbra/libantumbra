#!/bin/sh

set -e

rm -rf stage-osx
mkdir stage-osx
make os=darwin
cp -a libantumbra.framework stage-osx/
cp README.org stage-osx/

cd stage-osx
tar -czf ../BUILD-osx.tar.gz *
