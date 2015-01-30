#include "libantumbra.h"

static uint32_t unpacku32(const uint8_t *buf)
{
    return (uint32_t)buf[0] << 24 | (uint32_t)buf[1] << 16 | (uint32_t)buf[2] << 8 | buf[3];
}

static void packu32(uint32_t n, uint8_t *buf)
{
    buf[0] = n >> 24;
    buf[1] = n >> 16 & 0xff;
    buf[2] = n >> 8 & 0xff;
    buf[3] = n & 0xff;
}

AnError AnTemp_ReadRaw_S(AnCtx *ctx, AnDevice *dev, uint32_t *rawout)
{
    uint8_t inbuf[4];
    AnError err = AnCmd_Invoke_S(ctx, dev, AnTemp_API, AnTemp_CMD_READRAW,
                                 NULL, 0, inbuf, sizeof inbuf);
    if (err)
        return err;
    *rawout = unpacku32(inbuf);
    return AnError_SUCCESS;
}

AnError AnTemp_ReadTemp_S(AnCtx *ctx, AnDevice *dev, uint32_t *tempout)
{
    uint8_t inbuf[4];
    AnError err = AnCmd_Invoke_S(ctx, dev, AnTemp_API, AnTemp_CMD_READTEMP,
                                 NULL, 0, inbuf, sizeof inbuf);
    if (err)
        return err;
    *tempout = unpacku32(inbuf);
    return AnError_SUCCESS;
}

AnError AnTemp_ReadCal_S(AnCtx *ctx, AnDevice *dev, AnTempCal *calout)
{
    uint8_t inbuf[16];
    AnError err = AnCmd_Invoke_S(ctx, dev, AnTemp_API, AnTemp_CMD_READCAL,
                                 NULL, 0, inbuf, sizeof inbuf);
    if (err)
        return err;
    *calout = (AnTempCal){
        .a_sensor = unpacku32(inbuf),
        .a_temp = unpacku32(inbuf + 4),
        .b_sensor = unpacku32(inbuf + 8),
        .b_temp = unpacku32(inbuf + 12),
    };
    return AnError_SUCCESS;
}

AnError AnTemp_WriteCal_S(AnCtx *ctx, AnDevice *dev, const AnTempCal *calin)
{
    uint8_t outbuf[16];
    packu32(calin->a_sensor, outbuf);
    packu32(calin->a_temp, outbuf + 4);
    packu32(calin->b_sensor, outbuf + 8);
    packu32(calin->b_temp, outbuf + 12);
    return AnCmd_Invoke_S(ctx, dev, AnTemp_API, AnTemp_CMD_WRITECAL,
                          outbuf, sizeof outbuf, NULL, 0);
}
