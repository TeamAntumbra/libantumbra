#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <getopt.h>
#include <stdlib.h>
#include <errno.h>

#include "libantumbra.h"

#include "hsv.h"
#include "usage.h"

static AnCtx *ctx;
static AnLogLevel log_level;
static int device_num;

static const struct option options[] = {
    {"help", no_argument, NULL, 'h'},
    {"device", required_argument, NULL, 'd'},
    {"verbose", no_argument, NULL, 'v'},
    {"quiet", no_argument, NULL, 'q'},

    {NULL, 0, NULL, 0}
};

static void validate_flash_info(AnFlashInfo *info)
{
    if (info->pagesize > 0x4000) {
        An_LOG(ctx, AnLog_ERROR,
               "page size is unexpectedly large (>%d bytes)", 0x4000);
        exit(1);
    }

    if (info->numpages > 0x400) {
        An_LOG(ctx, AnLog_ERROR, "too many pages (>%d)", 0x400);
        exit(1);
    }
}

static uint8_t *read_all_flash(AnDevice *dev, AnFlashInfo *info)
{
    uint8_t *flash = malloc(info->pagesize * info->numpages);
    if (!flash) {
        An_LOG(ctx, AnLog_ERROR, "malloc failed");
        exit(1);
    }

    for (uint32_t ipage = 0; ipage < info->numpages; ++ipage) {
        if (AnFlash_ReadPage_S(ctx, dev, info, ipage,
                               flash + ipage * info->pagesize))
            exit(1);
    }

    return flash;
}

static void write_all_flash(AnDevice *dev, AnFlashInfo *info,
                            const uint8_t *flash, size_t srcsize)
{
    uint8_t page[info->pagesize];
    for (uint32_t off = 0, ipage = 0;
         off < srcsize;
         off += info->pagesize, ++ipage) {
        size_t realpagesize = (srcsize - off < info->pagesize ?
                               srcsize - off : info->pagesize);
        memcpy(page, flash + off, realpagesize);
        memset(page + realpagesize, 0, sizeof page - realpagesize);

        if (AnFlash_WritePage_S(ctx, dev, info, ipage, page))
            exit(1);
    }
}

static uint8_t *read_all_file(FILE *f, size_t *lenout)
{
    uint8_t *buf = NULL;
    size_t bufsz = 0, buflen = 0;

    size_t nread;
    uint8_t readbuf[512];
    while ((nread = fread(readbuf, 1, sizeof readbuf, f)) != 0) {
        if (buflen + nread > bufsz) {
            size_t newbufsz = bufsz > 0 ? 2 * bufsz : sizeof readbuf;
            if (newbufsz > 0x100000) {
                An_LOG(ctx, AnLog_ERROR,
                       "abort file read: size exceeds %d", 0x100000);
                exit(1);
            }

            uint8_t *newbuf = realloc(buf, newbufsz);
            if (!newbuf) {
                An_LOG(ctx, AnLog_ERROR, "realloc failed");
                exit(1);
            }

            buf = newbuf;
            bufsz = newbufsz;
        }

        memcpy(buf + buflen, readbuf, nread);
        buflen += nread;
    }
    if (ferror(f)) {
        An_LOG(ctx, AnLog_ERROR, "fread error");
        exit(1);
    }

    *lenout = buflen;
    return buf;
}

static void write_all_file(const uint8_t *buf, size_t len, FILE *f)
{
    if (fwrite(buf, 1, len, f) < len) {
        An_LOG(ctx, AnLog_ERROR, "short fwrite");
        exit(1);
    }
}

static uint8_t *read_eeprom(AnDevice *dev, AnEepromInfo *info)
{
    uint8_t *eeprom = malloc(info->size);
    if (!eeprom) {
        An_LOG(ctx, AnLog_ERROR, "malloc failed");
        exit(1);
    }

    for (uint16_t off = 0; off < info->size; off += 48) {
        uint8_t len = (info->size - off < 48 ?
                       info->size - off : 48);
        if (AnEeprom_Read_S(ctx, dev, info, off, len, eeprom + off))
            exit(1);
    }

    return eeprom;
}

static void write_eeprom(AnDevice *dev, AnEepromInfo *info,
                         const uint8_t *eeprom, uint16_t size)
{
    for (uint16_t off = 0; off < size; off += 48) {
        uint8_t len = (size - off < 48 ?
                       size - off : 48);
        if (AnEeprom_Write_S(ctx, dev, info, off, len, eeprom + off))
            exit(1);
    }
}

static FILE *try_open(const char *name, const char *mode)
{
    FILE *f = fopen(name, mode);
    if (!f) {
        fprintf(stderr, "%s: %s\n", name, strerror(errno));
        exit(1);
    }
    return f;
}

static void cmd_list(int argc, char **argv, AnDeviceInfo **devs, size_t ndevs)
{
    for (int i = 0; i < ndevs; ++i) {
        uint8_t bus, addr;
        uint16_t vid, pid;
        AnDeviceInfo_UsbInfo(devs[i], &bus, &addr, &vid, &pid);
        printf("%d: bus 0x%02x addr 0x%02x vid 0x%04x pid 0x%04x\n",
               i, (unsigned int)bus, (unsigned int)addr,
               (unsigned int)vid, (unsigned int)pid);
    }
}

static void cmd_flashread(int argc, char **argv, AnDevice *dev)
{
    FILE *outf = argc ? try_open(argv[0], "wb") : stdout;

    AnFlashInfo info;
    if (AnFlash_Info_S(ctx, dev, &info))
        exit(1);
    validate_flash_info(&info);

    uint8_t *flash = read_all_flash(dev, &info);
    write_all_file(flash, info.pagesize * info.numpages, outf);
    free(flash);

    if (outf != stdout)
        fclose(outf);
}

static void cmd_flashwrite(int argc, char **argv, AnDevice *dev)
{
    FILE *inf = argc ? try_open(argv[0], "rb") : stdin;

    AnFlashInfo info;
    if (AnFlash_Info_S(ctx, dev, &info))
        exit(1);
    validate_flash_info(&info);

    size_t size;
    uint8_t *flash = read_all_file(inf, &size);
    write_all_flash(dev, &info, flash, size);
    free(flash);

    if (inf != stdin)
        fclose(inf);
}

static void cmd_eepromread(int argc, char **argv, AnDevice *dev)
{
    FILE *outf = argc ? try_open(argv[0], "wb") : stdout;

    AnEepromInfo info;
    if (AnEeprom_Info_S(ctx, dev, &info))
        exit(1);

    uint8_t *eeprom = read_eeprom(dev, &info);
    write_all_file(eeprom, info.size, outf);
    free(eeprom);

    if (outf != stdout)
        fclose(outf);
}

static void cmd_eepromwrite(int argc, char **argv, AnDevice *dev)
{
    FILE *inf = argc ? try_open(argv[0], "rb") : stdin;

    AnEepromInfo info;
    if (AnEeprom_Info_S(ctx, dev, &info))
        exit(1);

    size_t size;
    uint8_t *eeprom = read_all_file(inf, &size);

    if (size > 0xffff) {
        An_LOG(ctx, AnLog_ERROR, "file exceeds max EEPROM size %d\n", 0xffff);
        exit(1);
    }

    write_eeprom(dev, &info, eeprom, size);
    free(eeprom);

    if (inf != stdin)
        fclose(inf);
}

static void cmd_bootset(int argc, char **argv, AnDevice *dev)
{
    bool ldrp;
    if (!strcmp(argv[0], "main"))
        ldrp = false;
    else if (!strcmp(argv[0], "loader"))
        ldrp = true;
    else {
        fputs(usage_msg, stderr);
        exit(1);
    }

    if (AnBoot_SetForceLoader_S(ctx, dev, ldrp))
        exit(1);
}

static void cmd_reset(int argc, char **argv, AnDevice *dev)
{
    if (AnCore_Reset_S(ctx, dev))
        exit(1);
}

static void cmd_lightset(int argc, char **argv, AnDevice *dev)
{
    AnLightInfo info;
    if (AnLight_Info_S(ctx, dev, &info))
        exit(1);

    if (AnLight_Set_S(ctx, dev, &info,
                      strtol(argv[0], NULL, 0),
                      strtol(argv[1], NULL, 0),
                      strtol(argv[2], NULL, 0)))
        exit(1);
}

#define CMD_NODEV 0
#define CMD_LISTONLY 1
#define CMD_USEDEV 2

typedef void (*call_nodev)(int argc, char **argv);
typedef void (*call_listonly)(int argc, char **argv, AnDeviceInfo **devs,
                              size_t ndevs);
typedef void (*call_usedev)(int argc, char **argv, AnDevice *dev);

struct cmd {
    const char *name;
    int type;
    int argc; /* -1 to ignore argc */
    void *call;
};

static const struct cmd commands[] = {
    {"list", CMD_LISTONLY, 0, &cmd_list},
    {"flash-read", CMD_USEDEV, -1, &cmd_flashread},
    {"flash-write", CMD_USEDEV, -1, &cmd_flashwrite},
    {"eeprom-read", CMD_USEDEV, -1, &cmd_eepromread},
    {"eeprom-write", CMD_USEDEV, -1, &cmd_eepromwrite},
    {"boot-set", CMD_USEDEV, 1, &cmd_bootset},
    {"reset", CMD_USEDEV, 0, &cmd_reset},
    {"light-set", CMD_USEDEV, 3, &cmd_lightset},
};

static const struct cmd *match_cmd(const char *name)
{
    for (int i = 0; i < sizeof commands / sizeof *commands; ++i) {
        if (!strcmp(commands[i].name, name))
            return &commands[i];
    }
    return NULL;
}

static void dispatch_cmd(char *name, int argc, char **argv)
{
    const struct cmd *cmd = match_cmd(name);
    if (!cmd) {
        fprintf(stderr, "unknown command: %s\n", name);
        exit(1);
    }

    if (cmd->argc != -1 && argc != cmd->argc) {
        fputs(usage_msg, stderr);
        exit(1);
    }

    size_t ndevs;
    AnDeviceInfo **devs;

    if (cmd->type != CMD_NODEV) {
        if (AnCtx_Init(&ctx)) {
            fputs("AnCtx init failed\n", stderr);
            exit(1);
        }
        AnLog_SetLogging(ctx, log_level, stderr);

        if (AnDevice_GetList(ctx, &devs, &ndevs))
            exit(1);
    }

    if (cmd->type == CMD_NODEV)
        (*(call_nodev)cmd->call)(argc, argv);

    else if (cmd->type == CMD_LISTONLY)
        (*(call_listonly)cmd->call)(argc, argv, devs, ndevs);

    else if (cmd->type == CMD_USEDEV) {
        AnDevice *dev;

        if (device_num >= ndevs) {
            fprintf(stderr, "device %d does not exist\n", device_num);
            exit(1);
        }

        if (AnDevice_Open(ctx, devs[device_num], &dev))
            exit(1);

        (*(call_usedev)cmd->call)(argc, argv, dev);

        AnDevice_Close(ctx, dev);
    }

    AnDevice_FreeList(devs);
    AnCtx_Deinit(ctx);
 }

int main(int argc, char **argv)
{
    device_num = 0;
    log_level = AnLog_WARN;
    for (int c; (c = getopt_long(argc, argv, "hd:vq", options, NULL)) != -1;) {
        switch(c) {
        case 'h':
            fputs(usage_msg, stdout);
            exit(0);
            break;
        case 'd':
            {
                char *endptr;
                device_num = strtol(optarg, &endptr, 10);
                if (endptr == optarg || *endptr || device_num < 0) {
                    fprintf(stderr, "invalid device number: %s\n", optarg);
                    exit(1);
                }
            }
            break;
        case 'v':
            ++log_level;
            break;
        case 'q':
            log_level = AnLog_NONE;
            break;
        default:
            fputs(usage_msg, stderr);
            exit(1);
            break;
        }
    }

    if (optind == argc) {
        fputs(usage_msg, stderr);
        exit(1);
    }

    dispatch_cmd(argv[optind], argc - optind - 1, argv + optind + 1);
    return 0;
}
