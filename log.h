#ifndef AN_LOG_H
#define AN_LOG_H

#include "ctx.h"

#include <stdarg.h>
#include <stdio.h>

#define An_LOG(ctx, fmt, ...) AnLog_Log((ctx), ("[%s:%d:%s] " fmt "\n"), __FILE__, \
                                        __LINE__, __func__, __VA_ARGS__)

void AnLog_Log(AnCtx *ctx, const char *fmt, ...);

void AnLog_SetLogFile(AnCtx *ctx, FILE *f);

#endif
