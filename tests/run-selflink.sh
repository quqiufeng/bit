#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)
BUILD=${BUILD:-"$ROOT/build"}
LINK="$BUILD/c1link"

"$LINK" "$BUILD/selflink-stage1" _start \
    "$BUILD/startup_sysv.o" "$BUILD/runtime1.o" \
    "$BUILD/compiler1.o" "$BUILD/syscalls_sysv.o"
"$BUILD/selflink-stage1" < "$ROOT/linux/runtime.c" > "$BUILD/selflink-runtime2.o"
"$BUILD/selflink-stage1" < "$ROOT/linux/compiler.c" > "$BUILD/selflink-compiler2.o"
cmp "$BUILD/runtime1.o" "$BUILD/selflink-runtime2.o"
cmp "$BUILD/compiler1.o" "$BUILD/selflink-compiler2.o"
"$LINK" "$BUILD/selflink-stage2" _start \
    "$BUILD/startup_sysv.o" "$BUILD/selflink-runtime2.o" \
    "$BUILD/selflink-compiler2.o" "$BUILD/syscalls_sysv.o"
cmp "$BUILD/selflink-stage1" "$BUILD/selflink-stage2"
"$BUILD/selflink-stage2" < "$ROOT/examples/hello.c" > "$BUILD/selflink-hello.o"
"$LINK" "$BUILD/selflink-hello" _start \
    "$BUILD/startup_sysv.o" "$BUILD/selflink-runtime2.o" \
    "$BUILD/selflink-hello.o" "$BUILD/syscalls_sysv.o"
"$BUILD/selflink-hello"
echo "PASS: c1link self-host chain (no system ld for stage1/stage2)"
