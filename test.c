#include <stdio.h>

#include "antumbra.h"

int main(int argc, char **argv)
{
    AnCtx *ctx;
    if (AnCtx_Init(&ctx)) {
        fputs("ctx init failed\n", stderr);
        return 1;
    }
    AnDevice_Populate(ctx);

    for (int i = 0; i < AnDevice_GetCount(ctx); ++i) {
        const char *ser;
        AnDevice *dev = AnDevice_Get(ctx, i);
        AnDevice_Info(dev, NULL, NULL, &ser);
        puts(ser);
        if (AnDevice_Open(ctx, dev)) {
            fputs("device open failed\n", stderr);
            return 1;
        }
        AnDevice_Close(ctx, dev);
        AnDevice_Free(dev);
    }

    AnCtx_Deinit(ctx);
    return 0;
}
