#ifndef PXPAT_H_
#define PXPAT_H_

enum pat_error {
    PXPAT_E_SUCCESS = 0,
    PXPAT_E_INVALID_PARAMETERS,
    PXPAT_E_MAP_ENTRIES_EXCEEDED
};

enum pat_limits {
    PXPAT_TGA_MAP_ENTRIES_MAX = 255
};

enum pat_filefmt {
    PXPAT_FFMT_TGA
};

struct pattern;

typedef unsigned char pat_getpx_func(struct pattern *, unsigned int); /* returns a color index from Pattern.rgb */

enum pat_flags {
    PXPAT_F_NONE = 0,
    PXPAT_F_TILE = 1 /* TODO: PXPAT_TILE */
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

void pat_gen(struct pattern *pat);
void pat_write_tga(struct pattern pat, FILE *fp, unsigned int pxsz);
pat_getpx_func *pat_get_getpx_by_name(const char *name);

struct pat_context {
    struct pattern pat;
    enum pat_filefmt ffmt;
    unsigned int pxsz, seed;
    enum pat_error err;
};

struct pat_context pat_ctx_new(enum pat_filefmt ffmt,
                               unsigned int w,
                               unsigned int h,
                               unsigned int pxsz,
                               const char *rgb_s[],
                               unsigned int nrgb,
                               unsigned int seed,
                               const char *getpx_name,
                               enum pat_flags f);

const char *pat_strerror(struct pat_context *ctx);

#endif /* PXPAT_H_ */
