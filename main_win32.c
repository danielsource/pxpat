#ifndef PXPAT_VERSION
#define PXPAT_VERSION "win32-indev"
#endif

#include <stdarg.h>
#include <stdio.h>
#include <time.h>
#include <windows.h>

#include "./pxpat.h"
#include "./pxpat_util.h"

const char *arg0;

static struct pat_context ctx;

static void cleanup(void) {
    pat_ctx_free(&ctx);
}

static void dbprint(const char *fmt, ...) {
    char s[1024];
    va_list ap;

    va_start(ap, fmt);
    vsprintf(s, fmt, ap);
    va_end(ap);
    OutputDebugString(s);
    fputs(s, stderr);
}

int WINAPI WinMain(HINSTANCE inst,
                   HINSTANCE previnst,
                   PSTR cmdline,
                   int cmdshow) {
    UNUSED(inst);
    UNUSED(previnst);
    UNUSED(cmdshow);

    dbprint("pxpat %s\n", PXPAT_VERSION);

    arg0 = __argv[0];
    if (atexit(cleanup) != 0)
        return 1;
    if (cmdline && cmdline[0] != '\0')
        die("Usage: %s", arg0);

    MessageBox(0, "Hello, world!", arg0,
               MB_OK|MB_ICONINFORMATION);
    return 0;
}
