#ifndef AN_DEVICE_H
#define AN_DEVICE_H

#include <stdint.h>
#include <libusb.h>

#define AnDeviceState_DEAD 0
#define AnDeviceState_LIVE 1

typedef struct {
    uint16_t vid;
    uint16_t pid;
    char serial[32];

    unsigned int state;
    libusb_device_handle *usb;
} AnDevice;



#endif
