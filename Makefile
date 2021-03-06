# Copyright (c) 2015 Antumbra
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.

CFLAGS := -Wall -std=gnu99 $(addcflags)
LDFLAGS :=
LDLIBS :=

objs = device.o error.o ctx.o log.o cmd.o \
	core.o flash.o boot.o eeprom.o light.o temp.o

rm_files = *.a *.o usage.c

.PHONY: all
all: libantumbra.a

.PHONY: clean
clean:
	-rm -r $(rm_files)

%.a:
	$(AR) rcs $@ $^

libantumbra.a: $(objs)

usage.c: usage.txt
	( \
	echo '#include "usage.h"'; \
	echo 'static const char _usage_msg[] = {'; \
	xxd -i < $< || exit $$?; \
	echo ', 0x00'; \
	echo '};'; \
	echo 'const char *usage_msg = _usage_msg;'; \
	) > $@

ifeq ($(os),win32)

CC = i686-w64-mingw32-gcc
AR = i686-w64-mingw32-ar
LD = i686-w64-mingw32-gcc
CFLAGS += -Ideps/win32-libusb

rm_files += *.exe *.dll

%.dll %.exe:
	$(LD) $(LDFLAGS) -o $@ $^ $(LDLIBS)

all: libantumbra.dll antumbratool.exe glowdrvinst.exe

$(objs): CFLAGS += -DANTUMBRA_WIN_DLLBUILD
libantumbra.dll: LDFLAGS += -shared -Wl,--out-implib=$@.a
libantumbra.dll: LDLIBS += -Ldeps/win32-libusb -lusb-1.0
libantumbra.dll: $(objs)

antumbratool.exe: LDLIBS += -lm -L. -lantumbra
antumbratool.exe: antumbratool.o usage.o

glowdrvinst.exe: CFLAGS += -Ideps/libwdi
glowdrvinst.exe: LDFLAGS += -Wl,--enable-stdcall-fixup,--allow-multiple-definition
glowdrvinst.exe: LDLIBS += -Ldeps/libwdi -lwdi
glowdrvinst.exe: glowdrvinst.o

else ifeq ($(os),linux)

CC = gcc
AR = ar
LD = gcc
CFLAGS += $(shell pkg-config libusb-1.0 --cflags)

rm_files += antumbratool *.so

%.so:
	$(LD) $(LDFLAGS) -o $@ $^ $(LDLIBS)
antumbratool:
	$(LD) $(LDFLAGS) -o $@ $^ $(LDLIBS)

all: libantumbra.so antumbratool

libantumbra.a: CFLAGS += -fPIC

libantumbra.so: CFLAGS += -fPIC
libantumbra.so: LDFLAGS += -shared -fPIC
libantumbra.so: LDLIBS += $(shell pkg-config libusb-1.0 --libs)
libantumbra.so: $(objs)

antumbratool: LDLIBS += -lm $(shell pkg-config libusb-1.0 --libs) -L. -lantumbra
antumbratool: antumbratool.o usage.o

else ifeq ($(os),darwin)

CC = gcc
AR = ar
LD = gcc
CFLAGS += -Ideps/osx-libusb -mmacosx-version-min=10.7
LDFLAGS += -mmacosx-version-min=10.7

rm_files += antumbratool *.dylib *.framework *.zip

%.dylib:
	$(LD) $(LDFLAGS) -o $@ $^ $(LDLIBS)
antumbratool:
	$(LD) $(LDFLAGS) -o $@ $^ $(LDLIBS)

all: libantumbra.dylib antumbratool libantumbra.framework

libantumbra.dylib: LDFLAGS += -dynamiclib
libantumbra.dylib: LDLIBS += -L. -lusb-special
libantumbra.dylib: $(objs) libusb-special.dylib
	$(CC) $(LDFLAGS) -o $@ $(objs) $(LDLIBS)

# libusb with modified install name. The standard libusb could potentially have
# any install name, which would make changing libantumbra.dylib's libusb
# dependency path difficult.
libusb-special.dylib: deps/osx-libusb/libusb.dylib
	cp $< $@
	chmod u+w $@
	install_name_tool -id libusb-DUMMY-NAME $@

antumbratool: LDLIBS += -lm -L. -lantumbra
antumbratool: antumbratool.o usage.o

libantumbra.framework: libantumbra.dylib libusb-special.dylib antumbratool
	mkdir $@ $@/Headers $@/Resources

	cp libantumbra.dylib $@/libantumbra
	install_name_tool \
		-change libusb-DUMMY-NAME @loader_path/libusb.dylib \
		-id @loader_path/../Frameworks/libantumbra.framework/libantumbra \
		$@/libantumbra

	cp libusb-special.dylib $@/libusb.dylib
# The install name doesn't actually matter unless someone tries to build against
# this, but it's best not to leave dummy names lying around. Someone might step
# on them.
	install_name_tool -id @loader_path/libusb.dylib $@/libusb.dylib

	cp antumbratool $@/antumbratool
	install_name_tool -change libantumbra.dylib @loader_path/libantumbra $@/antumbratool

	cp libantumbra.h $@/Headers/libantumbra.h
	cp Info.plist $@/Resources/Info.plist

else

$(error Specify platform by os=... on command line. Acceptable values are: win32, linux, darwin)

endif
