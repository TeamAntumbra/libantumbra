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

static void dispatch_cmd(char *cmd, int cmdargc, char **cmdargv)
{
    if (AnCtx_Init(&ctx)) {
        fputs("AnCtx init failed\n", stderr);
        exit(1);
    }
    AnLog_SetLogging(ctx, log_level, stderr);

    size_t ndevs;
    AnDeviceInfo **devs;
    if (AnDevice_GetList(ctx, &devs, &ndevs))
        exit(1);

    if (!strcmp(cmd, "list")) {
        for (int i = 0; i < ndevs; ++i) {
            uint8_t bus, addr;
            uint16_t vid, pid;
            AnDeviceInfo_UsbInfo(devs[i], &bus, &addr, &vid, &pid);
            printf("%d: bus 0x%02x addr 0x%02x vid 0x%04x pid 0x%04x\n",
                   i, (unsigned int)bus, (unsigned int)addr,
                   (unsigned int)vid, (unsigned int)pid);
        }
    }

    else if (!strcmp(cmd, "flash-read")) {
        FILE *outf = stdout;
        if (cmdargc) {
            outf = fopen(cmdargv[0], "wb");
            if (!outf) {
                fprintf(stderr, "%s: %s\n", cmdargv[0], strerror(errno));
                exit(1);
            }
        }

        if (device_num >= ndevs) {
            fprintf(stderr, "device %d does not exist\n", device_num);
            exit(1);
        }

        AnDevice *dev;
        AnDevice_Open(ctx, devs[device_num], &dev);
        AnFlashInfo flinfo;
        if (AnFlash_Info_S(ctx, dev, &flinfo))
            exit(1);

        validate_flash_info(&flinfo);

        uint8_t *flash = read_all_flash(dev, &flinfo);
        write_all_file(flash, flinfo.pagesize * flinfo.numpages, outf);
        free(flash);

        if (outf != stdout)
            fclose(outf);
        AnDevice_Close(ctx, dev);
    }

    else if (!strcmp(cmd, "flash-write")) {
        FILE *inf = stdin;
        if (cmdargc) {
            inf = fopen(cmdargv[0], "rb");
            if (!inf) {
                fprintf(stderr, "%s: %s\n", cmdargv[0], strerror(errno));
                exit(1);
            }
        }

        if (device_num >= ndevs) {
            fprintf(stderr, "device %d does not exist\n", device_num);
            exit(1);
        }

        AnDevice *dev;
        AnDevice_Open(ctx, devs[device_num], &dev);
        AnFlashInfo flinfo;
        if (AnFlash_Info_S(ctx, dev, &flinfo))
            exit(1);

        validate_flash_info(&flinfo);

        size_t flen;
        uint8_t *allfile = read_all_file(inf, &flen);
        write_all_flash(dev, &flinfo, allfile, flen);
        free(allfile);

        if (inf != stdout)
            fclose(inf);
        AnDevice_Close(ctx, dev);
    }

    else {
        fprintf(stderr, "unknown command: %s\n", cmd);
        exit(1);
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

/*     if (AnCtx_Init(&ctx)) { */
/*         fputs("ctx init failed\n", stderr); */
/*         return 1; */
/*     } */
/*     AnLog_SetLogging(ctx, loglvl, stderr); */

/*     size_t ndevs; */
/*     AnDeviceInfo **devs; */
/*     if (AnDevice_GetList(ctx, &devs, &ndevs)) */
/*         return 1; */

/*     if (ndevs) { */
/*         fputs("device(s) found; acting on first\n", stdout); */

/*         devfn(ctx, devs[0]); */
/*     } */
/*     else */
/*         fputs("no devices found; doing nothing\n", stderr); */

/*     AnDevice_FreeList(devs); */
/*     AnCtx_Deinit(ctx); */
/*     return 0; */
/* } */
