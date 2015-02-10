#include "libantumbra.h"
#include "internal.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <libusb.h>
#include <stdbool.h>

void AnDeviceInfo_UsbInfo(AnDeviceInfo *info,
                          uint8_t *bus, uint8_t *addr,
                          uint16_t *vid, uint16_t *pid)
{
    if (bus) *bus = info->bus;
    if (addr) *addr = info->addr;
    if (vid) *vid = info->devdes.idVendor;
    if (pid) *pid = info->devdes.idProduct;
}

void AnDevice_Info(AnDevice *dev, AnDeviceInfo **info)
{
    *info = &dev->info;
}

static AnError part_configure_device(AnCtx *ctx, AnDevice *dev);
static AnError part_alloc_node(AnCtx *ctx, AnDeviceInfo *info,
                               AnDevice **devout);
static AnError part_open_device(AnCtx *ctx, AnDeviceInfo *info,
                                AnDevice **devout);
static AnError part_configure_device(AnCtx *ctx, AnDevice *dev);
static AnError part_get_cfgdes(AnCtx *ctx, AnDevice *dev);
static AnError part_get_magic(AnCtx *ctx, AnDevice *dev);
static AnError part_parse_magic(AnCtx *ctx, AnDevice *dev);
static bool parse_magic(const char *magic, uint8_t *outep, uint8_t *inep,
                        const char **misc);

static AnError part_alloc_node(AnCtx *ctx, AnDeviceInfo *info,
                               AnDevice **devout)
{
    AnCtxDevList *newnode = malloc(sizeof *newnode);
    if (!newnode) {
        An_LOG(ctx, AnLog_ERROR, "malloc AnCtxDevList failed: %s",
               strerror(errno));
        return AnError_MALLOCFAILED;
    }

    int err = part_open_device(ctx, info, devout);
    if (err) {
        free(newnode);
        return err;
    }

    newnode->dev = *devout;
    newnode->next = ctx->opendevs;
    ctx->opendevs = newnode;
    return AnError_SUCCESS;
}

static AnError part_open_device(AnCtx *ctx, AnDeviceInfo *info,
                                AnDevice **devout)
{
    AnDevice *dev = malloc(sizeof *dev);
    if (!dev) {
        An_LOG(ctx, AnLog_ERROR, "malloc AnDevice failed: %s", strerror(errno));
        return AnError_MALLOCFAILED;
    }

    libusb_device_handle *udevh;
    int err = libusb_open(info->udev, &udevh);
    if (err) {
        An_LOG(ctx, AnLog_ERROR, "libusb_open failed: %s",
               libusb_strerror(err));
        free(dev);
        return AnError_LIBUSB;
    }

    dev->info = *info;
    dev->udevh = udevh;

    err = part_configure_device(ctx, dev);
    if (err) {
        libusb_close(udevh);
        free(dev);
        return err;
    }

    *devout = dev;
    return AnError_SUCCESS;
}

static AnError part_configure_device(AnCtx *ctx, AnDevice *dev)
{
    An_LOG(ctx, AnLog_DEBUG, "set configuration -1 to reset state");
    int err = libusb_set_configuration(dev->udevh, -1);
    if (err)
        An_LOG(ctx, AnLog_DEBUG,
               "libusb_set_configuration(-1) failed (not fatal): %s",
               libusb_strerror(err));

    An_LOG(ctx, AnLog_DEBUG, "set configuration 1");
    err = libusb_set_configuration(dev->udevh, 1);
    if (err) {
        An_LOG(ctx, AnLog_ERROR, "libusb_set_configuration(1): %s",
               libusb_strerror(err));
        return AnError_LIBUSB;
    }

    An_LOG(ctx, AnLog_DEBUG, "claim interface 0");
    err = libusb_claim_interface(dev->udevh, 0);
    if (err) {
        An_LOG(ctx, AnLog_ERROR, "libusb_claim_interface(0): %s",
               libusb_strerror(err));
        return AnError_LIBUSB;
    }

    return part_get_cfgdes(ctx, dev);
}

static AnError part_get_cfgdes(AnCtx *ctx, AnDevice *dev)
{
    struct libusb_config_descriptor *cfgdes;
    int err = libusb_get_active_config_descriptor(dev->info.udev, &cfgdes);
    if (err) {
        An_LOG(ctx, AnLog_ERROR, "libusb_get_active_config_descriptor: %s",
               libusb_strerror(err));
        return AnError_LIBUSB;
    }

    dev->cfgdes = cfgdes;

    err = part_get_magic(ctx, dev);
    if (err) {
        libusb_free_config_descriptor(cfgdes);
        return err;
    }

    return AnError_SUCCESS;
}

static AnError part_get_magic(AnCtx *ctx, AnDevice *dev)
{
    if (dev->cfgdes->bNumInterfaces < 1 ||
        dev->cfgdes->interface[0].num_altsetting < 1) {
        An_LOG(ctx, AnLog_ERROR, "device has no interfaces!");
        return AnError_LIBUSB;
    }

    struct libusb_interface_descriptor intdes = (
        dev->cfgdes->interface[0].altsetting[0]
    );

    if (!intdes.iInterface) {
        An_LOG(ctx, AnLog_ERROR, "device has no interface string descriptor");
        return AnError_LIBUSB;
    }

    int err = libusb_get_string_descriptor_ascii(
        dev->udevh, intdes.iInterface, (unsigned char *)dev->magic,
        sizeof dev->magic - 1
    );
    if (err < 0) {
        An_LOG(ctx, AnLog_ERROR, "libusb_get_string_descriptor_ascii: %s",
               libusb_strerror(err));
        return AnError_LIBUSB;
    }
    dev->magic[sizeof dev->magic - 1] = '\0';
    An_LOG(ctx, AnLog_INFO, "magic: %s", dev->magic);

    return part_parse_magic(ctx, dev);
}

static AnError part_parse_magic(AnCtx *ctx, AnDevice *dev)
{
    uint8_t epo, epi;
    const char *misc;

    if (!parse_magic(dev->magic, &epo, &epi, &misc)) {
        An_LOG(ctx, AnLog_ERROR, "invalid magic");
        return AnError_LIBUSB;
    }

    if (epo & 0x80 || ~epi & 0x80) {
        An_LOG(ctx, AnLog_ERROR, "invalid endpoint direction");
        return AnError_LIBUSB;
    }

    An_LOG(ctx, AnLog_DEBUG, "base protocol endpoints: OUT 0x%02x IN 0x%02x",
           epo, epi);

    dev->epo = epo;
    dev->epi = epi;
    return AnError_SUCCESS;
}

AnError AnDevice_Open(AnCtx *ctx, AnDeviceInfo *info, AnDevice **devout)
{
    An_LOG(ctx, AnLog_INFO,
           "open device: bus %03d addr %03d vid 0x%04x pid 0x%04x",
           info->bus, info->addr, info->devdes.idVendor,
           info->devdes.idProduct);
    return part_alloc_node(ctx, info, devout);
}

void AnDevice_Close(AnCtx *ctx, AnDevice *dev)
{
    An_LOG(ctx, AnLog_INFO,
           "close device: bus %03d addr %03d vid 0x%04x pid 0x%04x",
           dev->info.bus, dev->info.addr, dev->info.devdes.idVendor,
           dev->info.devdes.idProduct);
    AnCtxDevList *prev = NULL,
                 *cur = ctx->opendevs;
    while (cur) {
        if (cur->dev == dev) {
            AnDevice_InternalClose(ctx, cur->dev);
            if (prev)
                prev->next = cur->next;
            else
                ctx->opendevs = cur->next;
            free(cur);
            return;
        }
        prev = cur;
        cur = cur->next;
    }
    An_LOG(ctx, AnLog_ERROR, "device %p not found in ctx->opendevs", dev);
}

void AnDevice_InternalClose(AnCtx *ctx, AnDevice *dev)
{
    An_LOG(ctx, AnLog_DEBUG, "close USB handle for device %p", dev);
    libusb_free_config_descriptor(dev->cfgdes);
    /* Final deref on dev->info.udev */
    libusb_close(dev->udevh);
    free(dev);
}

static bool parse_magic(const char *magic, uint8_t *outep, uint8_t *inep,
                        const char **misc)
{
    /* Magic form as regex: io\.antumbra\.glowapi/([0-9a-fA-F]{2}/){2}.* */

    static const char prefix[] = "io.antumbra.glowapi/";
    if (strstr(magic, prefix) != magic)
        return false;

    const char *endpoints = magic + strlen(prefix);
    static const char hexd[] = "0123456789abcdefABCDEF";
    if (!(strchr(hexd, endpoints[0]) && strchr(hexd, endpoints[1]) &&
          endpoints[2] == '/' &&
          strchr(hexd, endpoints[3]) && strchr(hexd, endpoints[4]) &&
          endpoints[5] == '/'))
        return false;

    char outepstr[3], inepstr[3];
    memcpy(outepstr, endpoints, 2);
    memcpy(inepstr, endpoints + 3, 2);
    outepstr[2] = '\0';
    inepstr[2] = '\0';

    *outep = strtol(outepstr, NULL, 16),
    *inep = strtol(inepstr, NULL, 16);
    *misc = endpoints + 6;
    return true;
}

static AnError populate_info(AnCtx *ctx, AnDeviceInfo *info,
                             libusb_device *udev)
{
    int err = libusb_get_device_descriptor(udev, &info->devdes);
    if (err) {
        An_LOG(ctx, AnLog_ERROR, "libusb_get_device_descriptor: %s",
               libusb_strerror(err));
        return err;
    }

    info->bus = libusb_get_bus_number(udev);
    info->addr = libusb_get_device_address(udev);
    info->udev = udev;
    return AnError_SUCCESS;
}

static bool match_vid_pid(uint16_t vid, uint16_t pid)
{
    static const struct {uint16_t vid; uint16_t pid;} ids[] = {
        {0x03eb, 0x2040},
        {0x16d0, 0x0a85},
    };
    for (int i = 0; i < sizeof ids / sizeof *ids; ++i) {
        if (ids[i].vid == vid && ids[i].pid == pid)
            return true;
    }
    return false;
}

AnError AnDevice_GetList(AnCtx *ctx, AnDeviceInfo ***outdevs, size_t *outndevs)
{
    An_LOG(ctx, AnLog_DEBUG, "enumerate devices...");

    libusb_device **udevs;
    ssize_t ndevs = libusb_get_device_list(ctx->uctx, &udevs);
    if (ndevs < 0) {
        An_LOG(ctx, AnLog_ERROR, "libusb_get_device_list: %s",
               libusb_strerror(ndevs));
        return AnError_LIBUSB;
    }

    AnDeviceInfo **devs = malloc((ndevs + 1) * sizeof *devs);
    if (!devs) {
        An_LOG(ctx, AnLog_ERROR, "malloc: %s", strerror(errno));
        return AnError_MALLOCFAILED;
    }
    memset(devs, 0, (ndevs + 1) * sizeof *devs);

    size_t j = 0;
    for (ssize_t i = 0; i < ndevs; ++i) {
        libusb_device *udev = udevs[i];
        AnDeviceInfo info;

        An_LOG(ctx, AnLog_DEBUG, "device: bus %03d addr %03d",
               libusb_get_bus_number(udev), libusb_get_device_address(udev));

        if (populate_info(ctx, &info, udev))
            continue;

        An_LOG(ctx, AnLog_DEBUG, "vid 0x%04x pid 0x%04x",
               info.devdes.idVendor, info.devdes.idProduct);

        if (!match_vid_pid(info.devdes.idVendor, info.devdes.idProduct)) {
            An_LOG(ctx, AnLog_DEBUG, "  does not match Antumbra VID/PID");
            continue;
        }

        devs[j] = malloc(sizeof *devs[j]);
        if (!devs[j]) {
            An_LOG(ctx, AnLog_ERROR, "malloc: %s", strerror(errno));
            continue;
        }

        libusb_ref_device(udev);
        *devs[j] = info;
        ++j;
    }

    libusb_free_device_list(udevs, 1);
    *outdevs = devs;
    *outndevs = j;
    return AnError_SUCCESS;
}

void AnDevice_FreeList(AnDeviceInfo **devs)
{
    for (size_t i = 0; devs[i]; ++i) {
        libusb_unref_device(devs[i]->udev);
        free(devs[i]);
    }
    free(devs);
}

AnCtx *AnCtx_InitReturn(AnError *outerr)
{
    AnCtx *ctx = NULL;
    *outerr = AnCtx_Init(&ctx);
    return ctx;
}

void *AnDevice_GetOpaqueList(AnCtx *ctx, size_t *outndevs, AnError *outerr)
{
    AnDeviceInfo **devs = NULL;
    *outerr = AnDevice_GetList(ctx, &devs, outndevs);
    return devs;
}

AnDeviceInfo *AnDevice_IndexOpaqueList(void *devlist, size_t index)
{
    return ((AnDeviceInfo **)devlist)[index];
}

void AnDevice_FreeOpaqueList(void *devlist)
{
    AnDevice_FreeList(devlist);
}

AnDevice *AnDevice_OpenReturn(AnCtx *ctx, AnDeviceInfo *info, AnError *outerr)
{
    AnDevice *dev = NULL;
    *outerr = AnDevice_Open(ctx, info, &dev);
    return dev;
}
