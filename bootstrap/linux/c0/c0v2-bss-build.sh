#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../../.." && pwd)
BUILD=${BUILD:-"$ROOT/build"}
OUT="$BUILD/c0v2-bss"
mkdir -p "$OUT"
"$BUILD/c1-stage1" < "$ROOT/bootstrap/linux/c0/c0v2_bss.c" > "$OUT/c0v2-bss.o"
"$BUILD/c1link" "$OUT/c0v2-bss" _start "$BUILD/startup_sysv.o" "$BUILD/runtime2.o" "$OUT/c0v2-bss.o" "$BUILD/syscalls_sysv.o"
