#ifndef AN_CTX_H
#define AN_CTX_H

#include <libusb.h>

#include "error.h"

typedef struct AnCtx AnCtx;

/* Create a new context and return its pointer via `ctx`. */
An_DLL AnError AnCtx_Init(AnCtx **ctx);

/* Free resources and destroy context. */
An_DLL void AnCtx_Deinit(AnCtx *ctx);

#endif
