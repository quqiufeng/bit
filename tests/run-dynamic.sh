#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)
BUILD=${BUILD:-"$ROOT/build"}
LD=${LD:-ld}
"$BUILD/c1-stage2" < "$ROOT/tests/dynamic/plugin.c" > "$BUILD/dynamic-plugin.o"
"$BUILD/c1link" --shared "$BUILD/libanswer.so" "$BUILD/dynamic-plugin.o"
"$BUILD/c1-stage2" < "$ROOT/tests/dynamic/main.c" > "$BUILD/dynamic-main.o"
"$LD" -o "$BUILD/dynamic-main" \
    -dynamic-linker /lib64/ld-linux-x86-64.so.2 \
    -rpath "$BUILD" \
    "$BUILD/startup_sysv.o" "$BUILD/runtime2.o" \
    "$BUILD/dynamic-main.o" -L"$BUILD" -lanswer "$BUILD/syscalls_sysv.o"
"$BUILD/dynamic-main"
echo "PASS: dynamic executable against C1 shared library"
