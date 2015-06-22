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

AnError AnBoot_SetForceLoader_S(AnCtx *ctx, AnDevice *dev, bool ldrp)
{
    uint8_t flag = ldrp ? 1 : 0;
    return AnCmd_Invoke_S(ctx, dev, AnBoot_API, AnBoot_CMD_SET,
                          &flag, 1, NULL, 0);
}
