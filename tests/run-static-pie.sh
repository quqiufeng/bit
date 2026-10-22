#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)
BUILD=${BUILD:-"$ROOT/build"}
LINK="$BUILD/c1link"

"$LINK" --pie "$BUILD/pie-stage1" _start \
    "$BUILD/startup_sysv.o" "$BUILD/runtime1.o" \
    "$BUILD/compiler1.o" "$BUILD/syscalls_sysv.o"
"$BUILD/pie-stage1" < "$ROOT/linux/runtime.c" > "$BUILD/pie-runtime2.o"
"$BUILD/pie-stage1" < "$ROOT/linux/compiler.c" > "$BUILD/pie-compiler2.o"
cmp "$BUILD/runtime1.o" "$BUILD/pie-runtime2.o"
cmp "$BUILD/compiler1.o" "$BUILD/pie-compiler2.o"
"$LINK" --pie "$BUILD/pie-stage2" _start \
    "$BUILD/startup_sysv.o" "$BUILD/pie-runtime2.o" \
    "$BUILD/pie-compiler2.o" "$BUILD/syscalls_sysv.o"
cmp "$BUILD/pie-stage1" "$BUILD/pie-stage2"
"$BUILD/pie-stage2" < "$ROOT/examples/hello.c" > "$BUILD/pie-hello.o"
"$LINK" --pie "$BUILD/pie-hello" _start \
    "$BUILD/startup_sysv.o" "$BUILD/pie-runtime2.o" \
    "$BUILD/pie-hello.o" "$BUILD/syscalls_sysv.o"
"$BUILD/pie-hello"
echo "PASS: static PIE self-host chain"
