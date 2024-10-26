#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "./pxpat.h"
#include "./pxpat_util.h"

#define PAT_GETPX_FUNC(name) {#name, name}
static struct {
    const char *name;
    pat_getpx_func *getpx;
} pat_getpx_funcs[] = {
    PAT_GETPX_FUNC(pat_getpx),
    PAT_GETPX_FUNC(pat_getpx_simple_rand),
    PAT_GETPX_FUNC(pat_getpx_grid),
    PAT_GETPX_FUNC(pat_getpx_cycle_colors),
    PAT_GETPX_FUNC(pat_getpx_checkpat),
    PAT_GETPX_FUNC(pat_getpx_main_diag),
    {NULL, NULL}
};

unsigned char pat_getpx(struct pattern *pat, unsigned int pos) {
    return pat_getpx_simple_rand(pat, pos);
}


unsigned char pat_getpx_simple_rand(struct pattern *pat, unsigned int pos) {
    UNUSED(pos);
    return (unsigned char)(rand() % pat->nrgb);
}

static unsigned char pat_getpx_grid_(unsigned int x,
                                     unsigned int y,
                                     unsigned int h,
                                     unsigned char bg, /* background color */
                                     unsigned char fg, /* foreground color */
                                     unsigned char vx  /* vertex color */) {
    static const unsigned int rows = 32;
    const unsigned int cellsz = h/rows;

    if (cellsz == 0)
        return bg;
    x %= cellsz;
    y %= cellsz;
    if ((x == 1 && y == 0) ||
        (x == 0 && y == 1) ||
        (x == 1 && y == 1) ||
        (x == 2 && y == 1) ||
        (x == 1 && y == 2))
        return vx;
    if (x == 1 || y == 1)
        return fg;
    return bg;
}

unsigned char pat_getpx_grid(struct pattern *pat, unsigned int pos) {
    unsigned int x, y;

    x = pos % pat->w;
    y = pos / pat->w;
    if (pat->nrgb < 2)
        return pat_getpx_grid_(x, y, pat->h, 0, 0, 0);
    if (pat->nrgb == 2)
        return pat_getpx_grid_(x, y, pat->h, 0, 1, 1);
    return pat_getpx_grid_(x, y, pat->h, 0, 1, 2);
}

unsigned char pat_getpx_cycle_colors(struct pattern *pat, unsigned int pos) {
    return (unsigned char)(pos % pat->nrgb);
}

unsigned char pat_getpx_checkpat(struct pattern *pat, unsigned int pos) {
    unsigned int r, c;

    if (pat->nrgb < 2)
        return 0;
    r = pos / pat->w;
    c = pos % pat->w;
    return (r + c) % 2 == 0;
}

unsigned char pat_getpx_main_diag(struct pattern *pat, unsigned int pos) {
    unsigned int r, c;

    if (pat->nrgb < 2)
        return 0;
    r = pos / pat->w;
    c = pos % pat->w;
    return r == c;
}

void pat_gen(struct pattern *pat) {
    unsigned int i;

    assert(pat);
    assert(pat->getpx);
    assert(pat->w >= 1 && pat->h >= 1);
    assert(pat->nrgb >= 1);

    pat->data = ecalloc(pat->w * pat->h, 1);
    for (i = 0; i < pat->w * pat->h; ++i)
        pat->data[i] = pat->getpx(pat, i);
}

void pat_write_tga(struct pattern pat, FILE *fp, unsigned int pxsz) {
    unsigned int i;
    unsigned char r, g, b;

    /* TGA data type 1: color-mapped images */
    unsigned char header[18] = {
        /* 0*/ 0,          /* image identification field (IIF) size */
        /* 1*/ 1,          /* color map type */
        /* 2*/ 1,          /* image type */
        /* 3*/ 0, 0,       /* color map origin (lo-hi) */
        /* 5*/ 0, 0,       /* color map length (lo-hi) */
        /* 7*/ 24,         /* color map entry size */
        /* 8*/ 0, 0, 0, 0, /* image x,y origin (lo-hi) */
        /*12*/ 0, 0, 0, 0, /* image width,height (lo-hi) */
        /*16*/ 8,          /* image pixel size */
        /*17*/ 0,          /* image descriptor byte */
    };

    assert(fp);
    assert(pxsz >= 1); /* TODO: pxsz support */
    assert(pat.rgb);
    assert(pat.nrgb <= PXPAT_TGA_MAP_ENTRIES_MAX);

    header[5] = (unsigned char)pat.nrgb;
    header[12] = pat.w & 0xff;
    header[13] = (pat.w >> 8) & 0xff;
    header[14] = pat.h & 0xff;
    header[15] = (pat.h >> 8) & 0xff;
    header[17] |= 1 << 5; /* set top-left origin */
    fwrite(&header, sizeof header, 1, fp);

    /* color map data */
    for (i = 0; i < pat.nrgb; ++i) {
        r = (pat.rgb[i] >> 16) & 0xff;
        g = (pat.rgb[i] >> 8) & 0xff;
        b = pat.rgb[i] & 0xff;
        fputc(b, fp);
        fputc(g, fp);
        fputc(r, fp);
    }

    /* image data */
    for (i = 0; i < pat.w * pat.h; ++i)
        fputc(pat.data[i], fp);
}

pat_getpx_func *pat_get_getpx_by_name(const char *name) {
    unsigned int i;

    assert(name);

    for (i = 0; pat_getpx_funcs[i].getpx; ++i)
        if (strcmp(name, pat_getpx_funcs[i].name) == 0)
            return pat_getpx_funcs[i].getpx;
    return NULL;
}

struct pat_context pat_ctx_new(enum pat_filefmt ffmt,
                               unsigned int w,
                               unsigned int h,
                               unsigned int pxsz,
                               const char *rgb_s[],
                               unsigned int nrgb,
                               unsigned int seed,
                               const char *getpx_name,
                               enum pat_flags f) {
    struct pat_context ctx = {0};
    unsigned int i;

    assert(rgb_s);
    assert(nrgb > 0);
    assert(ffmt == PXPAT_FFMT_TGA);

    if (w <= 0 || h <= 0 || pxsz <= 0) {
        ctx.err = PXPAT_E_INVALID_PARAMETERS;
        return ctx;
    }
    switch (ffmt) {
    case PXPAT_FFMT_TGA:
        if (nrgb > PXPAT_TGA_MAP_ENTRIES_MAX) {
            ctx.err = PXPAT_E_MAP_ENTRIES_EXCEEDED;
            return ctx;
        }
        break;
    }

    ctx.ffmt = ffmt;
    ctx.pxsz = pxsz;
    ctx.seed = seed;
    ctx.pat.w = w;
    ctx.pat.h = h;
    ctx.pat.nrgb = nrgb;
    ctx.pat.getpx = pat_getpx;
    ctx.pat.f = f;

    srand(ctx.seed);

    if (getpx_name) {
        ctx.pat.getpx = pat_get_getpx_by_name(getpx_name);
        if (!ctx.pat.getpx)
            ctx.pat.getpx = pat_getpx;
    }

    ctx.pat.rgb = ecalloc(ctx.pat.nrgb, sizeof *ctx.pat.rgb);
    for (i = 0; i < ctx.pat.nrgb; ++i) {
        if (rgb_s[i][0] == '#')
            ++rgb_s[i];
        ctx.pat.rgb[i] = strtoul(rgb_s[i], NULL, 16);
    }

    pat_gen(&ctx.pat);
    return ctx;
}

void pat_ctx_write(struct pat_context *ctx, FILE *fp) {
    assert(ctx);
    assert(fp);
    assert(ctx->ffmt == PXPAT_FFMT_TGA);

    switch (ctx->ffmt) {
    case PXPAT_FFMT_TGA:
        pat_write_tga(ctx->pat, fp, ctx->pxsz);
        break;
    }
}

void pat_ctx_free(struct pat_context *ctx) {
    if (!ctx)
        return;
    free(ctx->pat.rgb);
    free(ctx->pat.data);
}

const char *pat_strerror(struct pat_context *ctx) {
    static char unk[] = "Unknown error 0000";

    assert(ctx);

    switch (ctx->err) {
    case 0:
        return "Success";
    case PXPAT_E_INVALID_PARAMETERS:
        return "Some of the passed parameters are invalid";
    case PXPAT_E_MAP_ENTRIES_EXCEEDED:
        return "The number of colors exceeded the limit";
    default:
        if (ctx->err >= 0)
            sprintf(unk + sizeof(unk) - 4, "%u", ctx->err % 1000);
        else
            sprintf(unk + sizeof(unk) - 5, "%u", ctx->err % 1000);
        return unk;
    }
}
