#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "./pxpat_util.h"

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
