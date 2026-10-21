#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)
BUILD=${BUILD:-"$ROOT/build"}

"$BUILD/c1-stage1" < "$ROOT/linux/linker.c" > "$BUILD/linker1.o"
"$BUILD/c1link" "$BUILD/c1link1" _start \
    "$BUILD/startup_sysv.o" "$BUILD/runtime1.o" \
    "$BUILD/linker1.o" "$BUILD/syscalls_sysv.o"

"$BUILD/c1-stage2" < "$ROOT/linux/linker.c" > "$BUILD/linker2.o"
cmp "$BUILD/linker1.o" "$BUILD/linker2.o"
"$BUILD/c1link1" "$BUILD/c1link2" _start \
    "$BUILD/startup_sysv.o" "$BUILD/runtime2.o" \
    "$BUILD/linker2.o" "$BUILD/syscalls_sysv.o"
cmp "$BUILD/c1link1" "$BUILD/c1link2"

"$BUILD/c1link2" "$BUILD/linker-selfhost-hello" _start \
    "$BUILD/startup_sysv.o" "$BUILD/runtime2.o" \
    "$BUILD/hello.o" "$BUILD/syscalls_sysv.o"
"$BUILD/linker-selfhost-hello"
echo "PASS: c1link self-hosted (c1link1 = c1link2)"
