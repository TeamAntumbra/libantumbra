#include "libantumbra.h"

AnError AnCore_Ask_S(AnCtx *ctx, AnDevice *dev,
                     uint32_t api, bool *supp)
{
    uint8_t apienc[4] = {api >> 24,
                         api >> 16 & 0xff,
                         api >> 8 & 0xff,
                         api & 0xff};
    uint8_t supbyte;
    AnError err = AnCmd_Invoke_S(ctx, dev,
                                 AnCore_API, AnCore_CMD_ASK,
                                 apienc, sizeof apienc,
                                 &supbyte, 1);
    if (!err)
        *supp = supbyte != 0;
    return err;
}
