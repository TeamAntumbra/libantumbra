#include <stdio.h>
#include <libusb.h>

#include "libantumbra.h"

struct AnCtx {
    AnLogLevel loglevel;
    FILE *logf;
    libusb_context *uctx;

    int ndevs;
    AnDevice **devs;
};
