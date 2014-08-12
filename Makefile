CFLAGS := $(addcflags) -Wall -std=gnu99
LDFLAGS :=
LDLIBS :=

# Building on Windows requires libusb static library and headers to be present
# in the source tree.

ifeq ($(os),win)
	CC = i686-w64-mingw32-gcc
	AR = i686-w64-mingw32-ar
	CFLAGS += -Ilibusb/include/libusb-1.0 -D'An_DLL=__cdecl __declspec(dllexport)'
	LDLIBS += -static -Llibusb/MinGW32/static -lusb-1.0
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
