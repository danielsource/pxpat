#!/bin/sh

set -e

: "${CC:=cc}"
comp_flags='-std=c89 -g -Og -Wall -Wextra -Wpedantic'

mkdir -p build
$CC $comp_flags pxpat.c -o build/pxpat.bin
