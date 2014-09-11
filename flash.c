#include "libantumbra.h"

#include <string.h>

AnError AnFlash_Info_S(AnCtx *ctx, AnDevice *dev, AnFlashInfo *info)
{
    uint8_t rsp[6];
    AnError err = AnCmd_Invoke_S(ctx, dev,
                                 AnFlash_API, AnFlash_CMD_INFO,
                                 NULL, 0,
                                 &rsp, sizeof rsp);
    if (!err) {
        info->pagesize = rsp[0] << 8 | rsp[1];
        info->numpages = rsp[2] << 24 | rsp[3] << 16 | rsp[4] << 8 | rsp[5];
    }
    return err;
}

static AnError read_page(AnCtx *ctx, AnDevice *dev,
                         uint32_t pageidx)
{
    uint8_t arg[4] = {pageidx >> 24,
                      pageidx >> 16 & 0xff,
                      pageidx >> 8 & 0xff,
                      pageidx & 0xff};
    uint8_t rdst;
    AnError err = AnCmd_Invoke_S(ctx, dev,
                                 AnFlash_API, AnFlash_CMD_PAGEREAD,
                                 arg, sizeof arg,
                                 &rdst, 1);
    if (err)
        return err;
    if (rdst)
        return AnError_CMDFAILURE;
    return AnError_SUCCESS;
}

static AnError write_page(AnCtx *ctx, AnDevice *dev,
                          uint32_t pageidx)
{
    uint8_t arg[4] = {pageidx >> 24,
                      pageidx >> 16 & 0xff,
                      pageidx >> 8 & 0xff,
                      pageidx & 0xff};
    uint8_t rdst;
    AnError err = AnCmd_Invoke_S(ctx, dev,
                                 AnFlash_API, AnFlash_CMD_PAGEWRITE,
                                 arg, sizeof arg,
                                 &rdst, 1);
    if (err)
        return err;
    if (rdst)
        return AnError_CMDFAILURE;
    return AnError_SUCCESS;
}

static AnError read_buffer(AnCtx *ctx, AnDevice *dev,
                           uint16_t off, uint8_t len, uint8_t *out)
{
    if (len > 48) {
        An_LOG(ctx, AnLog_ERROR, "page buffer read is max 48 bytes");
        return AnError_OUTOFRANGE;
    }

    uint8_t arg[3] = {off >> 8, off & 0xff, len};
    uint8_t rsp[56];
    AnError err = AnCmd_Invoke_S(ctx, dev,
                                 AnFlash_API, AnFlash_CMD_BUFREAD,
                                 arg, sizeof arg,
                                 rsp, sizeof rsp);
    if (err)
        return err;
    if (rsp[0])
        return AnError_CMDFAILURE;

    memcpy(out, rsp + 8, len);
    return AnError_SUCCESS;
}

static AnError write_buffer(AnCtx *ctx, AnDevice *dev,
                            uint16_t off, uint8_t len, const uint8_t *in)
{
    if (len > 48) {
        An_LOG(ctx, AnLog_ERROR, "page buffer write is max 48 bytes");
        return AnError_OUTOFRANGE;
    }

    uint8_t arg[56] = {[0] = off >> 8, [1] = off & 0xff, [2] = len};
    memcpy(arg + 8, in, len);
    uint8_t rspst;
    AnError err = AnCmd_Invoke_S(ctx, dev,
                                 AnFlash_API, AnFlash_CMD_BUFWRITE,
                                 arg, sizeof arg,
                                 &rspst, 1);
    if (err)
        return err;
    if (rspst)
        return AnError_CMDFAILURE;

    return AnError_SUCCESS;
}

#define min(a, b) ((a) < (b) ? (a) : (b))

AnError AnFlash_ReadPage_S(AnCtx *ctx, AnDevice *dev,
                           AnFlashInfo *info,
                           uint32_t pageidx, uint8_t *page)
{
    AnError err;

    /* Load page to page buffer. */
    err = read_page(ctx, dev, pageidx);
    if (err)
        return err;

    /* Read out page buffer. */
    for (uint16_t off = 0; off < info->pagesize;) {
        uint8_t len = min(48, info->pagesize - off);

        err = read_buffer(ctx, dev, off, len, page);
        if (err)
            return err;

        off += len;
        page += len;
    }

    return AnError_SUCCESS;
}

AnError AnFlash_WritePage_S(AnCtx *ctx, AnDevice *dev,
                            AnFlashInfo *info,
                            uint32_t pageidx, const uint8_t *page)
{
    AnError err;

    /* Load page buffer. */
    for (uint16_t off = 0; off < info->pagesize;) {
        uint8_t len = min(48, info->pagesize - off);

        err = write_buffer(ctx, dev, off, len, page);
        if (err)
            return err;

        off += len;
        page += len;
    }

    /* Write page buffer to page. */
    err = write_page(ctx, dev, pageidx);
    if (err)
        return err;

    return AnError_SUCCESS;
}
