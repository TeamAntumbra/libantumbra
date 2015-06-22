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

#include <stdlib.h>

AnError AnCtx_Init(AnCtx **ctx)
{
    AnCtx *newctx = malloc(sizeof *newctx);
    if (!newctx)
        return AnError_MALLOCFAILED;

    if (libusb_init(&newctx->uctx)) {
        free(newctx);
        return AnError_LIBUSB;
    }

    newctx->loglevel = AnLog_NONE;
    newctx->logf = NULL;
    newctx->opendevs = NULL;

    *ctx = newctx;
    return AnError_SUCCESS;
}

AnCtx *AnCtx_InitReturn(AnError *outerr)
{
    AnCtx *ctx = NULL;
    *outerr = AnCtx_Init(&ctx);
    return ctx;
}

void AnCtx_Deinit(AnCtx *ctx)
{
    AnCtxDevList *cur = ctx->opendevs;
    while (cur) {
        AnCtxDevList *next = cur->next;
        AnDevice_InternalClose(ctx, cur->dev);
        free(cur);
        cur = next;
    }

    libusb_exit(ctx->uctx);
    free(ctx);
}
