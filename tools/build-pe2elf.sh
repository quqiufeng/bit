#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
CC=${CC:-gcc}
SRC="$ROOT/tools/pe2elf.c"
HDR="$ROOT/tools/pe2elf_stubs.h"
OUT=${PE2ELF_BIN:-"$ROOT/build/pe2elf"}
if [ ! -x "$OUT" ] || [ "$SRC" -nt "$OUT" ] || [ "$HDR" -nt "$OUT" ]; then
    mkdir -p "$(dirname -- "$OUT")"
    tmp="$OUT.tmp.$$"
    trap 'rm -f "$tmp"' EXIT HUP INT TERM
    "$CC" -std=c99 -O2 -Wall -Wextra -o "$tmp" "$SRC"
    mv "$tmp" "$OUT"
    trap - EXIT HUP INT TERM
fi
printf '%s\n' "$OUT"
