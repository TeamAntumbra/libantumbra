#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include <math.h>
#include <string.h>

#include "antumbra.h"

#include "hsv.c"

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

        uint8_t col[3];
        float deg = 0;

        uint8_t outb[8];
        memset(outb, 0, sizeof outb);

        while (1) {
            hsv2rgb(deg, 1, 1, col, col + 1, col + 2);
            deg = fmodf(deg + 1, 360);

            memcpy(outb, col, sizeof col);
            if (AnDevice_SendBulkPacket_S(ctx, dev, sizeof outb, outb) ==
                AnError_DISCONNECTED)
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
