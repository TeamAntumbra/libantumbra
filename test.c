#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include <math.h>
#include <string.h>

#include "libantumbra.h"

#include "hsv.h"

static void plugfn(AnCtx *ctx, AnDeviceInfo *info)
{
    AnDevice *dev;
    if (!AnDevice_Open(ctx, info, &dev)) {
        AnDevice_Close(ctx, dev);
    }
}

int main(int argc, char **argv)
{
    AnCtx *ctx;
    if (AnCtx_Init(&ctx)) {
        fputs("ctx init failed\n", stderr);
        return 1;
    }
    AnLog_SetLogging(ctx, AnLog_DEBUG, stderr);

    AnDevicePlug_SetPlugFn(ctx, &plugfn);
    AnDevicePlug_Update(ctx);

    AnCtx_Deinit(ctx);
    return 0;
}
