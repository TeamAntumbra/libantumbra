#include "libantumbra.h"

AnError AnBoot_SetForceLoader_S(AnCtx *ctx, AnDevice *dev, bool ldrp)
{
    uint8_t flag = ldrp ? 1 : 0;
    return AnCmd_Invoke_S(ctx, dev, AnBoot_API, AnBoot_CMD_SET,
                          &flag, 1, NULL, 0);
}
