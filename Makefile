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

ifeq ($(os),win)
	CC = i686-w64-mingw32-gcc
	AR = i686-w64-mingw32-ar
	CFLAGS += -Ilibusb -D'An_DLL=__cdecl __declspec(dllexport)'
	LDLIBS += -static -Llibusb -lusb-1.0
	SOEXT = dll
	EXEXT = .exe
else
	CC = gcc
	AR = ar
	CFLAGS += -fPIC $(shell pkg-config libusb-1.0 --cflags) -D'An_DLL='
	LDFLAGS += $(shell pkg-config libusb-1.0 --libs)
	SOEXT = so
	EXEXT =
endif

objs = device.o error.o ctx.o log.o

all: libantumbra.$(SOEXT) libantumbra.a

.PHONY: clean
clean:
	-rm test *.so *.dll *.exe *.a *.o

%.$(SOEXT):
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

%.a:
	$(AR) rcs $@ $^

libantumbra.$(SOEXT): LDFLAGS += $(LIBUSB_LIBS) -shared
libantumbra.$(SOEXT): $(objs)

libantumbra.a: $(objs)

test: LDFLAGS += -lm
test: test.o hsv.o libantumbra.a
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

test.exe: LDFLAGS += -lm
test.exe: LDLIBS := -L. -lantumbra $(LDLIBS)
test.exe: test.o hsv.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)
