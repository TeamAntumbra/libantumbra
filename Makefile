CFLAGS := -Wall -std=c99 $(shell pkg-config libusb-1.0 --cflags)

objs = antumbra.o

all: libantumbra.so libantumbra.a

.PHONY: clean
clean:
	-rm *.so *.a *.o

%.so:
	$(LD) $(LDFLAGS) -o $@ $^

%.a:
	$(AR) rcs $@ $^

libantumbra.so: CFLAGS += -fPIC
libantumbra.so: LDFLAGS += $(shell pkg-config libusb-1.0 --libs) -shared
libantumbra.so: $(objs)

libantumbra.a: $(objs)
