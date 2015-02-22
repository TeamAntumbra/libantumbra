#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <direct.h>

#include <libwdi.h>

void die(int st)
{
    printf("Press ENTER to exit.\n");
    while (getchar() != '\n');
    exit(st);
}

int main(int argc, char **argv)
{
    int elevated = argc == 2 && !strcmp(argv[1], "elevated");

    struct wdi_device_info *list;
    struct wdi_options_create_list listopts = {0};
    struct wdi_options_prepare_driver prepopts = {0};
    prepopts.driver_type = WDI_WINUSB;
    prepopts.vendor_name = "Antumbra";

    int err = wdi_create_list(&list, &listopts);
    if (err && err != WDI_ERROR_NO_DEVICE) {
        fprintf(stderr, "wdi_create_list: %s\n", wdi_strerror(err));
        die(1);
    }

    for (struct wdi_device_info *dev = list; dev; dev = dev->next) {
        if (dev->vid == 0x16d0 && dev->pid == 0x0a85) {
            if (!elevated) {
                char buf[MAX_PATH];
                GetModuleFileName(NULL, buf, sizeof buf);
                printf("Executable is: %s\n", buf);
                int ret = ShellExecute(NULL, "runas", buf, "elevated", NULL, 5);
                exit(ret > 32 ? 0 : 1);
            }

            err = wdi_prepare_driver(dev, "glowdriver", "antumbraglow.inf", &prepopts);
            if (err) {
                fprintf(stderr, "wdi_prepare_driver: %s\n", wdi_strerror(err));
                die(1);
            }

            err = wdi_install_driver(dev, "glowdriver", "antumbraglow.inf", NULL);
            if (err) {
                fprintf(stderr, "wdi_install_driver: %s\n", wdi_strerror(err));
                die(1);
            }

            printf("installed WinUSB driver for VID 0x16d0 PID 0x0a85\n");
            die(0);
        }
    }

    printf("There are no devices that do not have a driver installed. Driver installation is not required.\n");
    wdi_destroy_list(list);
    exit(0);

    return 0;
}
