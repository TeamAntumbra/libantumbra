#include "libantumbra.h"

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
