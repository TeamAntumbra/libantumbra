#ifndef AN_INTERNAL_H
#define AN_INTERNAL_H

#include <stdio.h>
#include <stdint.h>
#include <libusb.h>

#include "libantumbra.h"

typedef struct AnCtxDevList AnCtxDevList;
struct AnCtxDevList {
    AnDevice *dev;
    AnCtxDevList *next;
};

struct AnCtx {
    AnLogLevel loglevel;
    FILE *logf;
    libusb_context *uctx;
    AnDevicePlugFn plugfn;
    AnCtxDevList *opendevs;
};

struct AnDeviceInfo {
    uint8_t bus;
    uint8_t addr;

    uint8_t vid;
    uint8_t pid;

    libusb_device *udev;
};

struct AnDevice {
    AnDeviceInfo info;
    libusb_device_handle *udevh;
};

void AnDevice_InternalClose(AnDevice *dev);

#endif
