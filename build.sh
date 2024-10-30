#!/bin/sh

set -e

: "${CC:=cc}"
comp_flags='-std=c89 -g -Og -Wall -Wextra -Wpedantic '$CFLAGS

if [ "$OS" = Windows_NT ]; then
    ext=.exe
else
    ext=
fi

mkdir -p build
$CC $comp_flags build.c -o build/pxpat${ext} $LDFLAGS
