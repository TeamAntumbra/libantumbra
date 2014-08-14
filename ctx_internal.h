#include <stdio.h>
#include <libusb.h>

#include "antumbra.h"

struct AnCtx {
    AnLogLevel loglevel;
    FILE *logf;
    libusb_context *uctx;

    int ndevs;
    AnDevice **devs;
};
