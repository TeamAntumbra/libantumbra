#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include <math.h>
#include <string.h>

#include "libantumbra.h"

#include "hsv.h"

static void devfn(AnCtx *ctx, AnDeviceInfo *info)
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

    size_t ndevs;
    AnDeviceInfo **devs;
    if (AnDevice_GetList(ctx, &devs, &ndevs))
        return 1;

    for (size_t i = 0; i < ndevs; ++i)
        devfn(ctx, devs[i]);

    AnDevice_FreeList(devs);
    AnCtx_Deinit(ctx);
    return 0;
}
