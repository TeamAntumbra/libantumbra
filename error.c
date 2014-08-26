#include "libantumbra.h"

const char err_outofrange[] = "(error message not defined)";

static const char *err_strings[] = {
    [AnError_SUCCESS] = "Success",
    [AnError_DISCONNECTED] = "Disconnected",
    [AnError_MALLOCFAILED] = "Memory allocation failed",
    [AnError_LIBUSB] = "libusb error",
    [AnError_WRONGSTATE] = "Device in wrong state for operation",
    [AnError_OUTOFRANGE] = "Value out of range",
};

const char *AnError_String(AnError e)
{
    return (0 <= e && e < sizeof err_strings / sizeof *err_strings
            ? err_strings[e] : err_outofrange);
}
