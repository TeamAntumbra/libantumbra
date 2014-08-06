#include <stdio.h>
#include <libusb.h>

struct AnCtx {
    FILE *logf;
    libusb_context *uctx;
    libusb_device **udevs;
};
