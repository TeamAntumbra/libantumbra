#ifndef AN_LIBANTUMBRA_H
#define AN_LIBANTUMBRA_H

#include <stdio.h>
#include <stdint.h>

#ifdef ANTUMBRA_WINDOWS
#define An_DLL __cdecl __declspec(dllexport)
#else
#define An_DLL
#endif

/* All is peachy. */
#define AnError_SUCCESS 0
/* Device evaporated while our back was turned. */
#define AnError_DISCONNECTED 1
/* Couldn't allocate memory. */
#define AnError_MALLOCFAILED 2
/* libusb choked. */
#define AnError_LIBUSB 3
/* Device in inapplicable state for operation. */
#define AnError_WRONGSTATE 4
/* Index or size out of range. */
#define AnError_OUTOFRANGE 5

/* Error value. Zero is success, nonzero is flaming death. */
typedef int AnError;

/* Get string message for error. */
An_DLL const char *AnError_String(AnError e);

typedef struct AnCtx AnCtx;

/* Create a new context and return its pointer via `ctx`. */
An_DLL AnError AnCtx_Init(AnCtx **ctx);

/* Free resources and destroy context. */
An_DLL void AnCtx_Deinit(AnCtx *ctx);

/* Disconnected / no longer available. */
#define AnDeviceState_DEAD 0
/* Handle open, but no interface in use. */
#define AnDeviceState_IDLE 1
/* Open and interface in use. */
#define AnDeviceState_OPEN 2

typedef struct AnDeviceInfo AnDeviceInfo;

An_DLL void AnDeviceInfo_UsbInfo(AnDeviceInfo *info,
                                 uint8_t *bus, uint8_t *addr,
                                 uint16_t *vid, uint16_t *pid);

typedef struct AnDevice AnDevice;

An_DLL void AnDevice_Info(AnDevice *dev, AnDeviceInfo **info);

An_DLL AnError AnDevice_Open(AnCtx *ctx, AnDeviceInfo *info, AnDevice **devout);

An_DLL void AnDevice_Close(AnCtx *ctx, AnDevice *dev);

/* Callback function pointer type for hotplug events.

   dev is only valid during invocation of the callback. */
typedef void (*AnDevicePlugFn)(AnCtx *ctx, AnDeviceInfo *dev);

/* Set hotplug callback. */
An_DLL void AnDevicePlug_SetPlugFn(AnCtx *ctx, AnDevicePlugFn fn);

/* Update device list and fire hotplug events. For best results, this function
   should be called at a regular interval. Internally, every invocation
   enumerates all USB devices, so rapid polling is not recommended. */
An_DLL AnError AnDevicePlug_Update(AnCtx *ctx);

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
An_DLL void AnLog_Log(AnCtx *ctx, AnLogLevel lvl, const char *fmt, ...);

/* Set minimum level and output file for logging, or NULL to disable. */
An_DLL void AnLog_SetLogging(AnCtx *ctx, AnLogLevel lvl, FILE *f);

/* Return a sigil (DD/II/WW/EE) for a given error level, or ?? for unknown
   level. */
An_DLL const char *AnLogLevel_Sigil(AnLogLevel lvl);

#endif
