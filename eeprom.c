#include "libantumbra.h"

#include <string.h>

AnError AnEeprom_Info_S(AnCtx *ctx, AnDevice *dev, AnEepromInfo *info)
{
    uint8_t rsp[2];
    AnError err = AnCmd_Invoke_S(ctx, dev, AnEeprom_API, AnEeprom_CMD_INFO,
                                 NULL, 0, rsp, sizeof rsp);
    if (!err)
        info->size = rsp[0] << 8 | rsp[1];
    return err;
}

AnError AnEeprom_Read_S(AnCtx *ctx, AnDevice *dev, AnEepromInfo *info,
                        uint16_t off, uint8_t len, uint8_t *out)
{
    if (len > 48) {
        An_LOG(ctx, AnLog_ERROR, "EEPROM read is max 48 bytes");
        return AnError_OUTOFRANGE;
    }
    uint8_t arg[3] = {off >> 8, off & 0xff, len};
    uint8_t rsp[56];
    AnError err = AnCmd_Invoke_S(ctx, dev, AnEeprom_API, AnEeprom_CMD_READ,
                                 arg, sizeof arg, rsp, sizeof rsp);
    if (err)
        return err;
    if (rsp[0]) {
        An_LOG(ctx, AnLog_ERROR, "EEPROM API error 0x%02x",
               (unsigned int)rsp[0]);
        return AnError_CMDFAILURE;
    }

    memcpy(out, rsp + 8, len);
    return AnError_SUCCESS;
}

AnError AnEeprom_Write_S(AnCtx *ctx, AnDevice *dev, AnEepromInfo *info,
                         uint16_t off, uint8_t len, const uint8_t *in)
{
    if (len > 48) {
        An_LOG(ctx, AnLog_ERROR, "EEPROM write is max 48 bytes");
        return AnError_OUTOFRANGE;
    }
    uint8_t arg[56] = {[0] = off >> 8, [1] = off & 0xff, [2] = len};
    memcpy(arg + 8, in, len);
    uint8_t rspst;
    AnError err = AnCmd_Invoke_S(ctx, dev, AnEeprom_API, AnEeprom_CMD_WRITE,
                                 arg, sizeof arg, &rspst, 1);
    if (err)
        return err;
    if (rspst) {
        An_LOG(ctx, AnLog_ERROR, "EEPROM API error 0x%02x",
               (unsigned int)rspst);
        return AnError_CMDFAILURE;
    }

    return AnError_SUCCESS;
}
