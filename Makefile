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
CFLAGS += -Ilibusb

dynamiclib: libantumbra.dylib
testprog: test

libantumbra.dylib: LDFLAGS += -dynamiclib
libantumbra.dylib: LDLIBS += -Llibusb -lusb
libantumbra.dylib: $(objs) libusb/libusb.dylib
	$(CC) $(LDFLAGS) -o $@ $(objs) $(LDLIBS)

# Standard libusb from external source (Homebrew, ...). This copy is not
# modified, but it must be present so that a modified version can be created. A
# symlink is acceptable.
libusb/libusb.dylib:
	@>&2 echo "$@ must be added manually! See README."
	@exit 1

# libusb with modified install name. The standard libusb could potentially have
# any install name, which would make changing libantumbra.dylib's libusb
# dependency path difficult.
libusb/libusb-special.dylib: libusb/libusb.dylib
	cp $< $@
	chmod u+w $@
	install_name_tool -id libusb-DUMMY-NAME $@

test: LDLIBS += -lm -lusb-1.0
test: test.o hsv.o libantumbra.a

all: libantumbra.framework libantumbra.framework.zip
clean: cleanfr
libantumbra.framework: libantumbra.dylib libusb/libusb-special.dylib
	mkdir $@ $@/Headers $@/Resources

	cp libantumbra.dylib $@/libantumbra
	install_name_tool \
		-change libusb-DUMMY-NAME @loader_path/libusb.dylib \
		-id /Library/Frameworks/libantumbra.framework/libantumbra \
		$@/libantumbra

	cp libusb/libusb-special.dylib $@/libusb.dylib
# The install name doesn't actually matter unless someone tries to build against
# this, but it's best not to leave dummy names lying around. Someone might step
# on them.
	install_name_tool -id @loader_path/libusb.dylib $@/libusb.dylib

	cp antumbra.h $@/Headers/libantumbra.h
	cp Info.plist $@/Resources/Info.plist
libantumbra.framework.zip: libantumbra.framework
	zip -r $@ $<
.PHONY: cleanfr
cleanfr:
	-rm libusb/libusb-special.dylib
	-rm -r libantumbra.framework libantumbra.framework.zip

else

$(error Specify architecture by os=<arch> on command line. <arch> is one of: win32, linux, darwin)

endif

staticlib: libantumbra.a
libantumbra.a: $(objs)
