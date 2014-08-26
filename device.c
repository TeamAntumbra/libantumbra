#include "libantumbra.h"
#include "internal.h"

#include <stdlib.h>

void AnDevice_InternalClose(AnDevice *dev)
{
    free(dev);
}

void AnDevicePlug_SetPlugFn(AnCtx *ctx, AnDevicePlugFn fn)
{
    ctx->plugfn = fn;
}
