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
static char *env_seed, *env_getpx;

static void cleanup(void) {
    pat_ctx_free(&ctx);
    free(env_seed);
    free(env_getpx);
}

int main(int argc, const char *argv[]) {
    arg0 = argv[0];

    if (atexit(cleanup) != 0)
        return 1;

    if (argc < 5)
        die("pxpat " PXPAT_VERSION "\n"
            "Usage: %s width height pixel_size 0xRRGGBB 0xRRGGBB... > output_file.tga\n"
            "Environment variables:\n"
            "  PXPAT_SEED (to be passed to srand)\n"
            "  PXPAT_GETPX (default: pat_getpx)", arg0);
    env_seed = getenv_alloc("PXPAT_SEED");
    env_getpx = getenv_alloc("PXPAT_GETPX");

    ctx = pat_ctx_new(atoi(argv[1]),
                      atoi(argv[2]),
                      atoi(argv[3]),
                      argv + 4,
                      argc - 4,
                      env_seed ? strtoul(env_seed, NULL, 10) : (unsigned int)time(NULL),
                      env_getpx,
                      PXPAT_F_NONE);
    if (ctx.err)
        die("%s: %s", arg0, pat_strerror(&ctx));
    pat_ctx_write(&ctx, stdout, PXPAT_FFMT_TGA);
    if (ctx.err)
        die("%s: %s", arg0, pat_strerror(&ctx));
    return 0;
}
