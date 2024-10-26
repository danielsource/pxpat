#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define UNUSED(par) (void)(par)

#define EXIT_INVALID_PARAMETER (2)

#define TGA_MAP_ENTRIES_MAX (255)

struct pattern;

typedef unsigned char pat_getpx_func(struct pattern *, unsigned int); /* returns a color index from Pattern.rgb */

enum pat_flags {
    PXPAT_F_NONE = 0,
    PXPAT_F_TILE = 1
};

struct pattern {
    unsigned int w, h;
    unsigned char *data;
    unsigned long *rgb; /* for hex triplets (each using 3 bytes out of 4 bytes~) */
    unsigned int nrgb;
    pat_getpx_func *getpx;
    enum pat_flags f;
};

unsigned char pat_getpx(struct pattern *pat, unsigned int pos);
unsigned char pat_getpx_simple_rand(struct pattern *pat, unsigned int pos);
unsigned char pat_getpx_grid(struct pattern *pat, unsigned int pos);
unsigned char pat_getpx_cycle_colors(struct pattern *pat, unsigned int pos);
unsigned char pat_getpx_checkpat(struct pattern *pat, unsigned int pos);
unsigned char pat_getpx_main_diag(struct pattern *pat, unsigned int pos);

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

static const char *arg0;

void die(const char *fmt, ...) {
    va_list ap;

    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    if (fmt[0] && fmt[strlen(fmt)-1] == ':') {
        fputc(' ', stderr);
        perror(NULL);
    } else {
        fputc('\n', stderr);
    }
    exit(EXIT_FAILURE);
}

void *ecalloc(size_t nmemb, size_t size) {
    void *p;

    if (!(p = calloc(nmemb, size)))
        die("calloc:");
    return p;
}

char *getenv_alloc(const char *name) {
    char *s, *var;
    size_t len;

    var = getenv(name);
    if (!var)
        return NULL;
    len = strlen(var);
    s = ecalloc(len+1, 1);
    memcpy(s, var, len);
    return s;
}

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
    assert(pat.nrgb <= TGA_MAP_ENTRIES_MAX);

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

    for (i = 0; pat_getpx_funcs[i].getpx; ++i)
        if (strcmp(name, pat_getpx_funcs[i].name) == 0)
            return pat_getpx_funcs[i].getpx;
    return NULL;
}

int main(int argc, char *argv[]) {
    struct pattern pat;
    unsigned int pxsz;
    char *env_seed = NULL, *env_getpx = NULL;
    unsigned int seed;
    unsigned int i;

    arg0 = argv[0];
    if (argc < 5)
        die("Usage: %s width height pixel_size 0xRRGGBB 0xRRGGBB... > output_file.tga\n"
            "Environment variables:\n"
            "  PXPAT_SEED (to be passed to srand)\n"
            "  PXPAT_GETPX (default: pat_getpx)", arg0);
    /* TODO: PXPAT_TILE */
    env_seed = getenv_alloc("PXPAT_SEED");
    env_getpx = getenv_alloc("PXPAT_GETPX");
    seed = env_seed ? strtoul(env_seed, NULL, 10) : (unsigned int)time(NULL);
    srand(seed);
    ++argv;
    pat.w = atoi(*argv++);
    pat.h = atoi(*argv++);
    pxsz = atoi(*argv++);
    pat.nrgb = argc - 4;
    if (pat.w <= 0 || pat.h <= 0 || pxsz <= 0 ||
        pat.nrgb <= 0 || pat.nrgb > TGA_MAP_ENTRIES_MAX)
        return EXIT_INVALID_PARAMETER;
    pat.rgb = ecalloc(pat.nrgb, sizeof *pat.rgb);
    if (env_getpx) {
        pat.getpx = pat_get_getpx_by_name(env_getpx);
        if (!pat.getpx)
            pat.getpx = pat_getpx;
    } else {
        pat.getpx = pat_getpx;
    }
    for (i = 0; i < pat.nrgb; ++i) {
        if (argv[i][0] == '#')
            ++argv[i];
        pat.rgb[i] = strtoul(argv[i], NULL, 16);
    }
    pat_gen(&pat);
    pat_write_tga(pat, stdout, pxsz);
    free(pat.rgb);
    free(pat.data);
    free(env_seed);
    free(env_getpx);
    return EXIT_SUCCESS;
}
