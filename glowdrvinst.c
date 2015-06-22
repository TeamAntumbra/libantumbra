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

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <direct.h>
#include <stdbool.h>
#include <stdint.h>

#include <libwdi.h>

bool batch = false;

void die(int st)
{
    if (!batch) {
        printf("Press ENTER to exit.\n");
        for (int c; c = getchar(), c != '\n' && c != EOF;);
    }
    exit(st);
}

void install_driver(uint16_t vid, uint16_t pid)
{
    printf("Will install WinUSB driver for VID 0x%04x PID 0x%04x\n",
           (unsigned int)vid, (unsigned int)pid);

    struct wdi_options_prepare_driver prepopts = {
        .driver_type = WDI_WINUSB,
        .vendor_name = "Antumbra",
        .device_guid = NULL,
        .disable_cat = false,
        .disable_signing = false,
        .cert_subject = NULL,
        .use_wcid_driver = false,
    };
    struct wdi_device_info dev = {
        .next = NULL,
        .vid = vid,
        .pid = pid,
        .is_composite = false,
        .mi = 0,
        .desc = "Glow",
        .driver = NULL,
        .device_id = NULL,
        .hardware_id = NULL,
        .compatible_id = NULL,
        .upper_filter = NULL,
        .driver_version = 0,
    };
    int err;

    const char prefix[] = "glowinftmp";
    char tmpdir[sizeof prefix + 8 + 1];
    sprintf(tmpdir, "%s%04x%04x", prefix, (unsigned int)vid, (unsigned int)pid);

    err = wdi_prepare_driver(&dev, tmpdir, "glow.inf", &prepopts);
    if (err) {
        fprintf(stderr, "wdi_prepare_driver: %s\n", wdi_strerror(err));
        die(1);
    }

    err = wdi_install_driver(&dev, tmpdir, "glow.inf", NULL);
    if (err) {
        fprintf(stderr, "wdi_install_driver: %s\n", wdi_strerror(err));
        die(1);
    }

    fputs("Installation successful\n", stdout);
}

int main(int argc, char **argv)
{
    bool elevated;
    for (int i = 1; i < argc; ++i) {
        if (!strcmp(argv[i], "batch"))
            batch = true;
        if (!strcmp(argv[i], "elevated"))
            elevated = true;
    }

    fputs("Antumbra Glow WinUSB driver installer\n", stdout);

    if (!batch && !elevated) {
        fputs("Trying to relaunch with elevated privileges\n", stdout);

        char buf[MAX_PATH];
        GetModuleFileName(NULL, buf, sizeof buf);
        printf("Executable is: %s\n", buf);
        int ret = ShellExecute(NULL, "runas", buf, "elevated", NULL, 5);
        if (ret > 32) {
            fputs("Successfully relaunched; exiting unprivileged instance.\n", stdout);
            die(0);
        }
        else {
            fputs("Relaunch failed!\n", stdout);
            die(1);
        }
    }

    if (!batch) {
        fputs("Press ENTER to begin installation...\n", stdout);
        for (int c; (c = getchar()) != '\n';) {
            if (c == EOF)
                exit(0);
        }
    }

    install_driver(0x16d0, 0x0a85);

    die(0);
    return 0;
}
