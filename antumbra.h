#ifndef AN_ANTUMBRA_H
#define AN_ANTUMBRA_H

#ifdef ANTUMBRA_WINDOWS
#define An_DLL __cdecl __declspec(dllexport)
#else
#define An_DLL
#endif

#include "ctx.h"
#include "device.h"
#include "error.h"
#include "log.h"

#endif
