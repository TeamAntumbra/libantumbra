#include "libantumbra.h"
#include "internal.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <libusb.h>
#include <stdbool.h>

void AnDeviceInfo_UsbInfo(AnDeviceInfo *info,
                          uint8_t *bus, uint8_t *addr,
                          uint16_t *vid, uint16_t *pid)
{
    if (bus) *bus = info->bus;
    if (addr) *addr = info->addr;
    if (vid) *vid = info->vid;
    if (pid) *pid = info->pid;
}

void AnDevice_Info(AnDevice *dev, AnDeviceInfo **info)
{
    *info = &dev->info;
}

AnError AnDevice_Open(AnCtx *ctx, AnDeviceInfo *info, AnDevice **devout)
{
    An_LOG(ctx, AnLog_DEBUG, "open device from AnDeviceInfo %p", info);

    AnCtxDevList *newnode = malloc(sizeof *newnode);
    if (!newnode) {
        An_LOG(ctx, AnLog_ERROR, "malloc AnCtxDevList failed: %s",
               strerror(errno));
        return AnError_MALLOCFAILED;
    }

    AnDevice *dev = malloc(sizeof *dev);
    if (!dev) {
        An_LOG(ctx, AnLog_ERROR, "malloc AnDevice failed: %s", strerror(errno));
        free(newnode);
        return AnError_MALLOCFAILED;
    }

    libusb_device_handle *udevh;
    int err = libusb_open(info->udev, &udevh);
    if (err) {
        An_LOG(ctx, AnLog_ERROR, "libusb_open failed: %s",
               libusb_strerror(err));
        free(newnode);
        free(dev);
        return AnError_LIBUSB;
    }

    dev->info = *info;
    dev->udevh = udevh;
    newnode->dev = dev;
    newnode->next = ctx->opendevs;
    ctx->opendevs = newnode;
    *devout = dev;
    return AnError_SUCCESS;
}

void AnDevice_Close(AnCtx *ctx, AnDevice *dev)
{
    An_LOG(ctx, AnLog_DEBUG, "close device %p", dev);
    AnCtxDevList *prev = NULL,
                 *cur = ctx->opendevs;
    while (cur) {
        if (cur->dev == dev) {
            AnDevice_InternalClose(ctx, cur->dev);
            if (prev)
                prev->next = cur->next;
            else
                ctx->opendevs = cur->next;
            free(cur);
            return;
        }
        prev = cur;
        cur = cur->next;
    }
    An_LOG(ctx, AnLog_ERROR, "device %p not found in ctx->opendevs", dev);
}

void AnDevice_InternalClose(AnCtx *ctx, AnDevice *dev)
{
    An_LOG(ctx, AnLog_DEBUG, "close USB handle for device %p", dev);
    /* Final deref on dev->info.udev */
    libusb_close(dev->udevh);
    free(dev);
}

void AnDevicePlug_SetPlugFn(AnCtx *ctx, AnDevicePlugFn fn)
{
    ctx->plugfn = fn;
}

static bool already_open(AnCtx *ctx, AnDeviceInfo *info)
{
    AnCtxDevList *cur = ctx->opendevs;
    while (cur) {
        AnDeviceInfo *oinfo = &cur->dev->info;
        if (oinfo->bus == info->bus &&
            oinfo->addr == info->addr &&
            oinfo->vid == info->vid &&
            oinfo->pid == info->pid)
            return true;
        cur = cur->next;
    }
    return false;
}

AnError AnDevicePlug_Update(AnCtx *ctx)
{
    An_LOG(ctx, AnLog_DEBUG, "enumerate devices...");

    libusb_device **devlist;
    ssize_t ndevs = libusb_get_device_list(ctx->uctx, &devlist);
    if (ndevs < 0) {
        An_LOG(ctx, AnLog_ERROR, "libusb_get_device_list: %s",
               libusb_strerror(ndevs));
        return AnError_LIBUSB;
    }

    for (ssize_t i = 0; i < ndevs; ++i) {
        libusb_device *udev = devlist[i];
        uint8_t bus = libusb_get_bus_number(udev),
                addr = libusb_get_device_address(udev);

        An_LOG(ctx, AnLog_DEBUG, "device: bus %03d addr %03d", bus, addr);

        struct libusb_device_descriptor devdes;
        int err = libusb_get_device_descriptor(udev, &devdes);
        if (err) {
            An_LOG(ctx, AnLog_ERROR, "libusb_get_device_descriptor: %s",
                   libusb_strerror(err));
            continue;
        }

        AnDeviceInfo info = {.bus = libusb_get_bus_number(udev),
                             .addr = libusb_get_device_address(udev),
                             .vid = devdes.idVendor,
                             .pid = devdes.idProduct,
                             .udev = udev};

        An_LOG(ctx, AnLog_DEBUG, "  AnDeviceInfo %p: vid 0x%04x pid 0x%04x",
               &info, info.vid, info.pid);

        if (already_open(ctx, &info))
            An_LOG(ctx, AnLog_DEBUG, "  already open; ignore");
        else if (ctx->plugfn)
            (*ctx->plugfn)(ctx, &info);
    }

    libusb_free_device_list(devlist, 1);
    return AnError_SUCCESS;
}
