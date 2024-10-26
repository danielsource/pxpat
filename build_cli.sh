#!/bin/sh

set -e

: "${CC:=cc}"
comp_flags='-std=c89 -g -Og -Wall -Wextra -Wpedantic'

mkdir -p build/cli
$CC $comp_flags build_cli.c -o build/cli/pxpat.bin
