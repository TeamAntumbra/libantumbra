#include "libantumbra.h"
#include "internal.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <libusb.h>

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
