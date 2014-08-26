#include "libantumbra.h"
#include "internal.h"

#include <stdlib.h>

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
