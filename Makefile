CFLAGS = -Wall -std=c99 -fPIC
LDFLAGS = -lusb-1.0 -shared

all: libantumbra.so

.PHONY: clean
clean:
    -rm libantumbra.so *.o

libantumbra.so:
