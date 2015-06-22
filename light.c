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
#include "internal.h"

#include <libusb.h>

AnError AnLight_Info_S(AnCtx *ctx, AnDevice *dev, AnLightInfo *info)
{
    return AnCmd_Invoke_S(ctx, dev, AnLight_API, AnLight_CMD_GETENDPOINT,
                          NULL, 0, &info->endpoint, 1);
}

AnError AnLight_Set_S(AnCtx *ctx, AnDevice *dev, AnLightInfo *info,
                      uint16_t r, uint16_t g, uint16_t b)
{
    uint8_t packet[6] = {r >> 8, r & 0xff,
                         g >> 8, g & 0xff,
                         b >> 8, b & 0xff};
    int xout;
    int err = libusb_bulk_transfer(dev->udevh, info->endpoint,
                                   packet, sizeof packet, &xout, 1000);
    if (err) {
        An_LOG(ctx, AnLog_ERROR, "libusb_bulk_transfer: %s",
               libusb_strerror(err));
        return AnError_LIBUSB;
    }

    return AnError_SUCCESS;
}
