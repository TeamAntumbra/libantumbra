#ifndef AN_ERROR_H
#define AN_ERROR_H

#define AnError_SUCCESS 0
#define AnError_DISCONNECTED 1

/* Error value. Zero is success, nonzero is flaming death. */
typedef int AnError;

/* Get string message for error. */
const char *AnError_String(AnError e);

#endif
