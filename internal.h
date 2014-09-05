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
    AnCtxDevList *opendevs;
};

struct AnDeviceInfo {
    uint8_t bus;
    uint8_t addr;

    libusb_device *udev;
    struct libusb_device_descriptor devdes;
};

struct AnDevice {
    AnDeviceInfo info;
    struct libusb_config_descriptor *cfgdes;
    char magic[128];

    uint8_t epo;
    uint8_t epi;
    libusb_device_handle *udevh;
};

void AnDevice_InternalClose(AnCtx *ctx, AnDevice *dev);

#endif
