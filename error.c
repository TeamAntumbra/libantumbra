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

const char err_outofrange[] = "(error message not defined)";

static const char *err_strings[] = {
    [AnError_SUCCESS] = "Success",
    [AnError_DISCONNECTED] = "Disconnected",
    [AnError_MALLOCFAILED] = "Memory allocation failed",
    [AnError_LIBUSB] = "libusb error",
    [AnError_WRONGSTATE] = "Device in wrong state for operation",
    [AnError_OUTOFRANGE] = "Value out of range",
    [AnError_UNSUPPORTED] = "Command unsupported",
    [AnError_CMDFAILURE] = "Command failure",
    [AnError_PROTOERROR] = "Unspecified protocol error",
};

const char *AnError_String(AnError e)
{
    return (0 <= e && e < sizeof err_strings / sizeof *err_strings
            ? err_strings[e] : err_outofrange);
}
