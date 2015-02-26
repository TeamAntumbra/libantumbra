#!/bin/sh

set -e

rm -rf stage-win32
mkdir stage-win32
make os=win32
cp deps/win32-libusb/libusb-1.0.dll stage-win32/
cp libantumbra.dll libantumbra.dll.a libantumbra.h libantumbra.a antumbratool.exe stage-win32/
mkdir stage-win32/glowdrvinst
cp glowdrvinst.exe stage-win32/glowdrvinst/
cp deps/libwdi/libwdi.dll deps/libwdi/msvcr120.dll stage-win32/glowdrvinst/
cp README.org stage-win32/

cd stage-win32
bsdtar --format=zip -cf ../BUILD-win32.zip *
