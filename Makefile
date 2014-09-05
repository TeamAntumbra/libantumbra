CFLAGS := -Wall -std=gnu99 $(addcflags)
LDFLAGS :=
LDLIBS :=

objs = device.o error.o ctx.o log.o cmd.o

rm_files = *.a *.o

.PHONY: all
all: libantumbra.a

.PHONY: clean
clean:
	-rm -r $(rm_files)

%.a:
	$(AR) rcs $@ $^

libantumbra.a: $(objs)

ifeq ($(os),win32)

CC = i686-w64-mingw32-gcc
AR = i686-w64-mingw32-ar
LD = i686-w64-mingw32-gcc
CFLAGS += -Ilibusb -DANTUMBRA_WINDOWS

rm_files += *.exe *.dll

%.dll %.exe:
	$(LD) $(LDFLAGS) -o $@ $^ $(LDLIBS)

all: libantumbra.dll test.exe

libantumbra.dll: LDFLAGS += -shared -Wl,--out-implib=$@.a
libantumbra.dll: LDLIBS += -Llibusb -lusb-1.0
libantumbra.dll: $(objs)

test.exe: LDLIBS += -lm -L. -lantumbra
test.exe: test.o hsv.o

else ifeq ($(os),linux)

CC = gcc
AR = ar
LD = gcc
CFLAGS += $(shell pkg-config libusb-1.0 --cflags)

rm_files += test *.so

%.so:
	$(LD) $(LDFLAGS) -o $@ $^ $(LDLIBS)
test:
	$(LD) $(LDFLAGS) -o $@ $^ $(LDLIBS)

all: libantumbra.so test

libantumbra.a: CFLAGS += -fPIC

libantumbra.so: CFLAGS += -fPIC
libantumbra.so: LDFLAGS += -shared -fPIC
libantumbra.so: LDLIBS += $(shell pkg-config libusb-1.0 --libs)
libantumbra.so: $(objs)

test: LDLIBS += -lm $(shell pkg-config libusb-1.0 --libs) -L. -lantumbra
test: test.o hsv.o

else ifeq ($(os),darwin)

CC = gcc
AR = ar
LD = gcc
CFLAGS += -Ilibusb

rm_files += test *.dylib *.framework *.zip libusb/libusb-special.dylib

%.dylib:
	$(LD) $(LDFLAGS) -o $@ $^ $(LDLIBS)
test:
	$(LD) $(LDFLAGS) -o $@ $^ $(LDLIBS)

all: libantumbra.dylib test libantumbra.framework libantumbra.framework.zip

libantumbra.dylib: LDFLAGS += -dynamiclib
libantumbra.dylib: LDLIBS += -Llibusb -lusb-special
libantumbra.dylib: $(objs) libusb/libusb-special.dylib
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

libantumbra.framework: libantumbra.dylib libusb/libusb-special.dylib
	mkdir $@ $@/Headers $@/Resources

	cp libantumbra.dylib $@/libantumbra
	install_name_tool \
		-change libusb-DUMMY-NAME @loader_path/libusb.dylib \
		-id @loader_path/../Frameworks/libantumbra.framework/libantumbra \
		$@/libantumbra

	cp libusb/libusb-special.dylib $@/libusb.dylib
# The install name doesn't actually matter unless someone tries to build against
# this, but it's best not to leave dummy names lying around. Someone might step
# on them.
	install_name_tool -id @loader_path/libusb.dylib $@/libusb.dylib

	cp libantumbra.h $@/Headers/libantumbra.h
	cp Info.plist $@/Resources/Info.plist
libantumbra.framework.zip: libantumbra.framework
	zip -r $@ $<

else

$(error Specify platform by os=... on command line. Acceptable values are: win32, linux, darwin)

endif
