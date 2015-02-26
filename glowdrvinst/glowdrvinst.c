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

struct usbpair {
    uint16_t vid;
    uint16_t pid;
    bool didinstall;
};

struct usbpair usbids[] = {
    {0x16d0, 0x0a85, false},
};

struct usbpair *lookup_ids(uint16_t vid, uint16_t pid)
{
    for (int i = 0; i < sizeof usbids / sizeof *usbids; ++i) {
        if (usbids[i].vid == vid && usbids[i].pid == pid)
            return &usbids[i];
    }
    return NULL;
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

    struct wdi_device_info *list;
    struct wdi_options_create_list listopts = {0};
    struct wdi_options_prepare_driver prepopts = {0};
    prepopts.driver_type = WDI_WINUSB;
    prepopts.vendor_name = "Antumbra";

    fputs("Antumbra Glow WinUSB driver installer\n", stdout);

    int err = wdi_create_list(&list, &listopts);
    if (err && err != WDI_ERROR_NO_DEVICE) {
        fprintf(stderr, "wdi_create_list: %s\n", wdi_strerror(err));
        die(1);
    }
    if (err == WDI_ERROR_NO_DEVICE)
        list = NULL;

    bool didinstall = false;
    for (struct wdi_device_info *dev = list; dev; dev = dev->next) {
        printf("Found unassociated device: VID 0x%04x PID 0x%04x",
               (unsigned int)dev->vid, (unsigned int)dev->pid);

        struct usbpair *pair = lookup_ids(dev->vid, dev->pid);
        if (!pair) {
            fputs("Ignore non-Glow device", stdout);
            continue;
        }
        if (pair->didinstall) {
            fputs("Already installed for this VID/PID", stdout);
            continue;
        }

        /* Only try to elevate privileges if there is at least one device to
           install. */
        if (!batch && !elevated) {
            fputs("Trying to relaunch with elevated privileges", stdout);

            char buf[MAX_PATH];
            GetModuleFileName(NULL, buf, sizeof buf);
            printf("Executable is: %s\n", buf);
            int ret = ShellExecute(NULL, "runas", buf, "elevated", NULL, 5);
            die(ret > 32 ? 0 : 1);
        }

        static const char prefix[] = "glowinftmp";
        char tmpdir[sizeof prefix + 8 + 1];
        sprintf(tmpdir, "%s%04x%04x", prefix,
                (unsigned int)dev->vid & 0xffff, (unsigned int)dev->pid & 0xffff);

        err = wdi_prepare_driver(dev, tmpdir, "glow.inf", &prepopts);
        if (err) {
            fprintf(stderr, "wdi_prepare_driver: %s\n", wdi_strerror(err));
            die(1);
        }

        err = wdi_install_driver(dev, tmpdir, "glow.inf", NULL);
        if (err) {
            fprintf(stderr, "wdi_install_driver: %s\n", wdi_strerror(err));
            die(1);
        }

        printf("Installed WinUSB driver for VID 0x%04x PID 0x%04x\n",
               (unsigned int)dev->vid, (unsigned int)dev->pid);
        pair->didinstall = true;
        didinstall = true;
    }

    if (!didinstall)
        fputs("Did not install driver.\n"
              "Either no Glow devices are connected, or all connected devices are already installed.\n"
              "Note that driver installation requires at least one device to be connected.\n",
              stdout);

    wdi_destroy_list(list);
    die(0);
    return 0;
}
