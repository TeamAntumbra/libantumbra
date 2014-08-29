#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <libusb.h>

#include "libantumbra.h"
#include "internal.h"

static void log_hex(AnCtx *ctx, const void *in, size_t sz)
{
    if (ctx->loglevel < AnLog_DEBUG)
        return;

    char *hex = malloc(2 * sz + 1);
    if (!hex)
        return;

    for (size_t i = 0; i < sz; ++i)
        snprintf(hex + 2 * i, 3, "%02x", (unsigned int)((uint8_t *)in)[i]);

    An_LOG(ctx, AnLog_DEBUG, "%s", hex);
    free(hex);
}

AnError AnCmd_SendRaw_S(AnCtx *ctx, AnDevice *dev, const void *buf,
                        unsigned int sz)
{
    uint8_t fixbuf[64];
    if (sz > sizeof fixbuf) {
        An_LOG(ctx, AnLog_ERROR, "can't send packet of %u bytes (max %u)",
               sz, (unsigned int)sizeof fixbuf);
        return AnError_OUTOFRANGE;
    }
    memset(fixbuf, 0, sizeof fixbuf);
    memcpy(fixbuf, buf, sz);

    An_LOG(ctx, AnLog_DEBUG, "send packet:");
    log_hex(ctx, fixbuf, sizeof fixbuf);

    int xout;
    int err = libusb_bulk_transfer(dev->udevh, dev->epo, fixbuf, sizeof fixbuf,
                                   &xout, 1000);
    if (err) {
        An_LOG(ctx, AnLog_ERROR, "libusb_bulk_transfer: %s",
               libusb_strerror(err));
        return AnError_LIBUSB;
    }

    return AnError_SUCCESS;
}

AnError AnCmd_RecvRaw_S(AnCtx *ctx, AnDevice *dev, void *buf, unsigned int sz)
{
    uint8_t fixbuf[64];
    if (sz > sizeof fixbuf) {
        An_LOG(ctx, AnLog_ERROR, "can't receive packet of %u bytes (max %u)",
               sz, (unsigned int)sizeof fixbuf);
        return AnError_OUTOFRANGE;
    }
    memset(fixbuf, 0, sizeof fixbuf);

    int xin;
    int err = libusb_bulk_transfer(dev->udevh, dev->epi, fixbuf, sizeof fixbuf,
                                   &xin, 1000);
    if (err) {
        An_LOG(ctx, AnLog_ERROR, "libusb_bulk_transfer: %s",
               libusb_strerror(err));
        return AnError_LIBUSB;
    }

    An_LOG(ctx, AnLog_DEBUG, "receive packet:");
    log_hex(ctx, fixbuf, sizeof fixbuf);

    memcpy(buf, fixbuf, sz);
    return AnError_SUCCESS;
}
