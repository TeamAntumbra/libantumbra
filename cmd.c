/* Copyright (c) 2015 Antumbra

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program. If not, see <http://www.gnu.org/licenses/>. */

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
    if (buf)
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

    if (buf)
        memcpy(buf, fixbuf, sz);
    return AnError_SUCCESS;
}

#define STATUS_NOPROTOERR 0
#define STATUS_UNSUPPORTEDAPICMD 1

static const char *status_strs[] = {
    [STATUS_NOPROTOERR] = "No protocol-level error",
    [STATUS_UNSUPPORTEDAPICMD] = "Unsupported API or command",
};
static const char status_str_unknown[] = "???";

static const char *lookup_status_str(uint8_t status)
{
    return (status < sizeof status_strs / sizeof *status_strs
            ? status_strs[status] : status_str_unknown);
}

AnError AnCmd_Invoke_S(AnCtx *ctx, AnDevice *dev, uint32_t api, uint16_t cmd,
                       const void *cmddata, unsigned int cmddata_sz,
                       void *rspdata, unsigned int rspdata_sz)
{
    An_LOG(ctx, AnLog_DEBUG,
           "send command: api 0x%08x cmd 0x%04hx cmdlen %d rsplen %d",
           api, cmd, cmddata_sz, rspdata_sz);

    uint8_t fixbuf[64];
    if (cmddata_sz > 56) {
        An_LOG(ctx, AnLog_ERROR, "max cmd payload is 56");
        return AnError_OUTOFRANGE;
    }
    if (rspdata_sz > 56) {
        An_LOG(ctx, AnLog_ERROR, "max rsp payload is 56");
        return AnError_OUTOFRANGE;
    }
    memset(fixbuf, 0, sizeof fixbuf);

    fixbuf[0] = api >> 24;
    fixbuf[1] = api >> 16 & 0xff;
    fixbuf[2] = api >> 8 & 0xff;
    fixbuf[3] = api & 0xff;
    fixbuf[4] = cmd >> 8 & 0xff;
    fixbuf[5] = cmd & 0xff;

    if (cmddata)
        memcpy(fixbuf + 8, cmddata, cmddata_sz);

    AnError err = AnCmd_SendRaw_S(ctx, dev, fixbuf, sizeof fixbuf);
    if (err)
        return err;
    err = AnCmd_RecvRaw_S(ctx, dev, fixbuf, sizeof fixbuf);
    if (err)
        return err;

    if (fixbuf[0])
        An_LOG(ctx, AnLog_ERROR, "error status: 0x%02x (%s)",
               (unsigned int)fixbuf[0], lookup_status_str(fixbuf[0]));

    if (rspdata)
        memcpy(rspdata, fixbuf + 8, rspdata_sz);
    return (fixbuf[0] == STATUS_NOPROTOERR ? AnError_SUCCESS :
            fixbuf[0] == STATUS_UNSUPPORTEDAPICMD ? AnError_UNSUPPORTED :
            AnError_PROTOERROR);
}
