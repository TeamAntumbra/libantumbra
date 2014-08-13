CFLAGS := $(addcflags) -Wall -std=gnu99
LDFLAGS :=
LDLIBS :=

# Building for Windows requires libusb static library and header to be present
# under libusb/ in the source tree. Given the libusb 1.0.19 Windows binary
# distribution (libusb-1.0.19.7z), which looks like this...
#
#     include/libusb-1.0/libusb.h
#     MinGW32/static/libusb-1.0.a
#     [other things we don't care about]
#
# ...the libusb/ directory here must look like this:
#
#     libusb.h
#     libusb-1.0.a

objs = device.o error.o ctx.o log.o

.PHONY: all dynamiclib staticlib testprog
all: dynamiclib staticlib testprog

.PHONY: clean
clean:
	-rm test *.so *.dll *.exe *.a *.o

%.so %.dll:
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

%.a:
	$(AR) rcs $@ $^

%.exe:
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

ifeq ($(os),win32)

CC = i686-w64-mingw32-gcc
AR = i686-w64-mingw32-ar
CFLAGS += -Ilibusb -D'An_DLL=__cdecl __declspec(dllexport)'

dynamiclib: libantumbra.dll
testprog: test.exe

libantumbra.dll: LDFLAGS += -shared
libantumbra.dll: LDLIBS += -static -Llibusb -lusb-1.0
libantumbra.dll: libantumbra.a

test.exe: LDLIBS += -lm -static -Llibusb -lusb-1.0
test.exe: test.o hsv.o libantumbra.a

else ifeq ($(os),linux)

CC = gcc
AR = ar
CFLAGS += $(shell pkg-config libusb-1.0 --cflags) -D'An_DLL='

dynamiclib: libantumbra.so
testprog: test

libantumbra.a: CFLAGS += -fPIC

libantumbra.so: LDFLAGS += -shared -fPIC
libantumbra.so: LDLIBS += $(shell pkg-config libusb-1.0 --libs)
libantumbra.so: libantumbra.a

test: LDLIBS += -lm $(shell pkg-config libusb-1.0 --libs)
test: test.o hsv.o libantumbra.a

else

$(error Specify architecture by os=<arch> on command line. <arch> is one of: win32, linux)

endif

libantumbra.a: $(objs)
