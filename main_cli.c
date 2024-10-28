#ifndef PXPAT_VERSION
#define PXPAT_VERSION "cli-indev"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "./pxpat.h"
#include "./pxpat_util.h"

const char *arg0;

static struct pat_context ctx;
static char *env_seed, *env_tile, *env_getpx;

static void cleanup(void) {
    pat_ctx_free(&ctx);
    free(env_seed);
    free(env_tile);
    free(env_getpx);
}

int main(int argc, const char *argv[]) {
    unsigned int i;

    arg0 = argv[0];

    if (atexit(cleanup) != 0)
        return 1;

    if (argc < 5) {
        fprintf(stderr,
                "pxpat " PXPAT_VERSION "\n"
                "Usage: %s width height pixel_size 0xRRGGBB... > output.tga\n"
                "Example: pxpat 320 180 4 0x963068 0xa2a9b0 0x100a0e > myfile.tga\n"
                "Environment variables:\n"
                "  PXPAT_SEED  : unsigned int (passed to srand, default: time(NULL))\n"
                "  PXPAT_TILE  : boolean (true if set)\n"
                "  PXPAT_GETPX : one of these:\n    ", arg0);
        for (i = 0; pat_getpx_funcs[i].getpx; ++i)
            if (pat_getpx_funcs[i+1].getpx)
                fprintf(stderr, "\"%s\", ", pat_getpx_funcs[i].name);
            else
                fprintf(stderr, "\"%s\"\n", pat_getpx_funcs[i].name);
        return 1;
    }
    env_seed = getenv_alloc("PXPAT_SEED");
    env_tile = getenv_alloc("PXPAT_TILE");
    env_getpx = getenv_alloc("PXPAT_GETPX");

    ctx = pat_ctx_new(atoi(argv[1]),
                      atoi(argv[2]),
                      atoi(argv[3]),
                      argv + 4,
                      argc - 4,
                      env_seed ? strtoul(env_seed, NULL, 10) : (unsigned int)time(NULL),
                      env_getpx,
                      env_tile ? PXPAT_F_TILE : PXPAT_F_NONE);
    if (ctx.err)
        die("%s: %s", arg0, pat_strerror(&ctx));
    pat_ctx_write(&ctx, stdout, PXPAT_FFMT_TGA);
    if (ctx.err)
        die("%s: %s", arg0, pat_strerror(&ctx));
    return 0;
}
