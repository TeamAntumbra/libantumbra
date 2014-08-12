#!/bin/sh
make addcflags='-O0 -g' clean all &&
gcc -Wall -std=gnu99 -O0 -g $(pkg-config libusb-1.0 --cflags) -lusb-1.0 -lm hsv.c test.c libantumbra.a -o test &&
valgrind ./test
