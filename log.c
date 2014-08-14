#include "antumbra.h"
#include "ctx_internal.h"

#include <stdarg.h>

static const char *const sigils[] = {
    [AnLog_ERROR] = "E",
    [AnLog_WARN] = "W",
    [AnLog_INFO] = "I",
    [AnLog_DEBUG] = "D",
};
static const char unknown_sigil[] = "??";

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
