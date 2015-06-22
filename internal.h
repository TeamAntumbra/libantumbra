/* Copyright (c) 2015 Antumbra

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program. If not, see <http://www.gnu.org/licenses/>. */

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
