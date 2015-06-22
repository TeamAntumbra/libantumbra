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

#include "libantumbra.h"

#include <string.h>

AnError AnCore_Echo_S(AnCtx *ctx, AnDevice *dev,
                      const void *outdata, unsigned int outdata_sz,
                      void *indata, unsigned int indata_sz)
{
    return AnCmd_Invoke_S(ctx, dev, AnCore_API, AnCore_CMD_ECHO, outdata, outdata_sz, indata, indata_sz);
}

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

AnError AnCore_Diagnostic_S(AnCtx *ctx, AnDevice *dev,
                            void *diagdata, unsigned int diagdata_sz)
{
    return AnCmd_Invoke_S(ctx, dev, AnCore_API, AnCore_CMD_DIAGNOSTIC, NULL, 0, diagdata, diagdata_sz);
}

#define MIN(a_, b_) ({typeof (a_) a = (a_); typeof (b_) b = (b_); (a < b) ? a : b;})

AnError AnCore_ImplementationId_S(AnCtx *ctx, AnDevice *dev,
                                  char *idstr, unsigned int idstr_sz)
{
    char idbuf[57];
    AnError err = AnCmd_Invoke_S(ctx, dev, AnCore_API, AnCore_CMD_IMPLEMENTATIONID,
                                 NULL, 0, idbuf, sizeof idbuf - 1);
    idbuf[sizeof idbuf - 1] = 0;
    if (!err) {
        unsigned int ncopy = MIN(strlen(idbuf) + 1, idstr_sz);
        memcpy(idstr, idbuf, ncopy);
        if (ncopy > 0)
            idstr[ncopy - 1] = 0;
    }
    return err;
}

AnError AnCore_DeviceId_S(AnCtx *ctx, AnDevice *dev,
                          void *idout, unsigned int idout_sz)
{
    return AnCmd_Invoke_S(ctx, dev, AnCore_API, AnCore_CMD_DEVICEID, NULL, 0, idout, idout_sz);
}

AnError AnCore_HardwareId_S(AnCtx *ctx, AnDevice *dev,
                            char *idstr, unsigned int idstr_sz)
{
    char idbuf[57];
    AnError err = AnCmd_Invoke_S(ctx, dev, AnCore_API, AnCore_CMD_HARDWAREID,
                                 NULL, 0, idbuf, sizeof idbuf - 1);
    idbuf[sizeof idbuf - 1] = 0;
    if (!err) {
        unsigned int ncopy = MIN(strlen(idbuf) + 1, idstr_sz);
        memcpy(idstr, idbuf, ncopy);
        if (ncopy > 0)
            idstr[ncopy - 1] = 0;
    }
    return err;
}

AnError AnCore_Reset_S(AnCtx *ctx, AnDevice *dev)
{
    return AnCmd_Invoke_S(ctx, dev, AnCore_API, AnCore_CMD_RESET,
                          NULL, 0, NULL, 0);
}
