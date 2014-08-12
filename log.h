#ifndef AN_LOG_H
#define AN_LOG_H

#include "ctx.h"

#include <stdarg.h>
#include <stdio.h>

#define AnLog_NONE (-1)
#define AnLog_ERROR 0
#define AnLog_WARN 1
#define AnLog_INFO 2
#define AnLog_DEBUG 3

typedef int AnLogLevel;

/* Log message with file/line/func context. */
#define An_LOG(ctx, lvl, fmt, ...) AnLog_Log(       \
        (ctx), (lvl), ("[%s:%d:%s %s] " fmt "\n"), __FILE__, __LINE__,  \
        __func__, AnLogLevel_Sigil((lvl)), ##__VA_ARGS__)


/* Log message. */
void AnLog_Log(AnCtx *ctx, AnLogLevel lvl, const char *fmt, ...);

/* Set minimum level and output file for logging, or NULL to disable. */
void AnLog_SetLogging(AnCtx *ctx, AnLogLevel lvl, FILE *f);

/* Return a sigil (DD/II/WW/EE) for a given error level, or ?? for unknown
   level. */
const char *AnLogLevel_Sigil(AnLogLevel lvl);

#endif
