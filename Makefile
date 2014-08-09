override CFLAGS := $(CFLAGS) -Wall -std=gnu99 $(shell pkg-config libusb-1.0 --cflags)
LDFLAGS :=

objs = device.o error.o ctx.o log.o

all: libantumbra.so libantumbra.a

.PHONY: clean
clean:
	-rm *.so *.a *.o

%.so:
	$(CC) $(LDFLAGS) -o $@ $^

%.a:
	$(AR) rcs $@ $^

libantumbra.so: CFLAGS += -fPIC
libantumbra.so: LDFLAGS += $(shell pkg-config libusb-1.0 --libs) -shared
libantumbra.so: $(objs)

libantumbra.a: $(objs)
