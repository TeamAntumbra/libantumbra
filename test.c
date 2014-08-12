#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include <math.h>
#include <string.h>

#include "antumbra.h"

#include "hsv.h"

int main(int argc, char **argv)
{
    AnCtx *ctx;
    if (AnCtx_Init(&ctx)) {
        fputs("ctx init failed\n", stderr);
        return 1;
    }
    AnLog_SetLogging(ctx, AnLog_INFO, stderr);
    AnDevice_Populate(ctx);

    for (int i = 0; i < AnDevice_GetCount(ctx); ++i) {
        const char *ser;
        AnDevice *dev = AnDevice_Get(ctx, i);
        AnDevice_Info(dev, NULL, NULL, &ser);
        if (AnDevice_Open(ctx, dev)) {
            fputs("device open failed\n", stderr);
            return 1;
        }

        float deg = 0;
        while (1) {
            uint8_t r, g, b;
            hsv2rgb(deg, 1, 1, &r, &g, &b);
            deg = fmodf(deg + 1, 360);

            if (AnDevice_SetRGB_S(ctx, dev, r, g, b) == AnError_DISCONNECTED)
                return 1;

            struct timespec ts = {.tv_sec = 0, .tv_nsec = 10000000};
            nanosleep(&ts, NULL);
        }

        AnDevice_Close(ctx, dev);
        AnDevice_Free(dev);
    }

    AnCtx_Deinit(ctx);
    return 0;
}
