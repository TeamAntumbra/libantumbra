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

#include <stdarg.h>

static const char *const sigils[] = {
    [AnLog_ERROR] = "E",
    [AnLog_WARN] = "W",
    [AnLog_INFO] = "I",
    [AnLog_DEBUG] = "D",
};
static const char unknown_sigil[] = "?";

void AnLog_Log(AnCtx *ctx, AnLogLevel lvl, const char *fmt, ...)
{
    if (!ctx->logf || lvl > ctx->loglevel)
        return;
    va_list ap;
    va_start(ap, fmt);
    vfprintf(ctx->logf, fmt, ap);
    va_end(ap);
}

void AnLog_SetLogging(AnCtx *ctx, AnLogLevel lvl, FILE *f)
{
    ctx->loglevel = lvl;
    ctx->logf = f;
}

const char *AnLogLevel_Sigil(AnLogLevel lvl)
{
    return (0 <= lvl && lvl < sizeof sigils / sizeof *sigils
            ? sigils[lvl] : unknown_sigil);
}
