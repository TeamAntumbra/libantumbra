CFLAGS := $(addcflags) -Wall -std=gnu99
LDFLAGS :=
LDLIBS :=

objs = device.o error.o ctx.o log.o

.PHONY: all dynamiclib staticlib testprog
all: dynamiclib staticlib testprog

.PHONY: clean
clean:
	-rm test *.so *.dylib *.dll *.exe *.a *.o

%.so %.dll %.dylib %.exe:
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

%.a:
	$(AR) rcs $@ $^

ifeq ($(os),win32)

CC = i686-w64-mingw32-gcc
AR = i686-w64-mingw32-ar
CFLAGS += -Ilibusb -DANTUMBRA_WINDOWS

dynamiclib: libantumbra.dll
testprog: test.exe

libantumbra.dll: LDFLAGS += -shared
libantumbra.dll: LDLIBS += -static -Llibusb -lusb-1.0
libantumbra.dll: $(objs)

test.exe: LDLIBS += -lm -static -Llibusb -lusb-1.0
test.exe: test.o hsv.o libantumbra.a

else ifeq ($(os),linux)

CC = gcc
AR = ar
CFLAGS += $(shell pkg-config libusb-1.0 --cflags)

dynamiclib: libantumbra.so
testprog: test

libantumbra.a: CFLAGS += -fPIC

libantumbra.so: CFLAGS += -fPIC
libantumbra.so: LDFLAGS += -shared -fPIC
libantumbra.so: LDLIBS += $(shell pkg-config libusb-1.0 --libs)
libantumbra.so: $(objs)

test: LDLIBS += -lm $(shell pkg-config libusb-1.0 --libs)
test: test.o hsv.o libantumbra.a

else ifeq ($(os),darwin)

CC = gcc
AR = ar
CFLAGS += $(shell pkg-config libusb-1.0 --cflags)

dynamiclib: libantumbra.dylib
testprog: test

libantumbra.dylib: LDFLAGS += -dynamiclib -install_name @rpath/libantumbra.dylib -rpath @loader_path/
libantumbra.dylib: LDLIBS += -Llibusb -lusb-1.0
libantumbra.dylib: $(objs)

test: LDLIBS += -lm -L. -lantumbra -rpath @loader_path/
test: test.o hsv.o

all: libantumbra.framework libantumbra.framework.zip
clean: cleanfr
libantumbra.framework: libantumbra.dylib
	mkdir $@ $@/Headers $@/Resources
	cp libantumbra.dylib $@/libantumbra
	install_name_tool -id @rpath/libantumbra $@/libantumbra
	cp libusb/libusb-1.0.dylib $@/libusb-1.0.dylib
	cp antumbra.h $@/Headers/libantumbra.h
	cp Info.plist $@/Resources/Info.plist
libantumbra.framework.zip: libantumbra.framework
	zip -r $@ $<
.PHONY: cleanfr
cleanfr:
	-rm -r libantumbra.framework libantumbra.framework.zip

else

$(error Specify architecture by os=<arch> on command line. <arch> is one of: win32, linux, darwin)

endif

staticlib: libantumbra.a
libantumbra.a: $(objs)
