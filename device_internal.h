#include <stdint.h>
#include <libusb.h>

struct AnDevice {
    uint16_t vid;
    uint16_t pid;
    char serial[32];

    unsigned int state;
    libusb_device_handle *usb;
};
