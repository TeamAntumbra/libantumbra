#!/bin/sh

rm -rf stage-linux
mkdir stage-linux
make os=linux
cp libantumbra.so libantumbra.a libantumbra.h antumbratool glow-udev.rules README.org stage-linux/

cd stage-linux
tar -czf ../BUILD-linux.tar.gz *
