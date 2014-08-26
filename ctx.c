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
    newctx->plugfn = NULL;
    newctx->opendevs = NULL;

    *ctx = newctx;
    return AnError_SUCCESS;
}

void AnCtx_Deinit(AnCtx *ctx)
{
    libusb_exit(ctx->uctx);

    AnCtxDevList *cur = ctx->opendevs;
    while (cur) {
        AnCtxDevList *next = cur->next;
        AnDevice_InternalClose(cur->dev);
        free(cur);
        cur = next;
    }

    free(ctx);
}
