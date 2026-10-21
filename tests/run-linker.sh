#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)
BUILD=${BUILD:-"$ROOT/build"}
LINK="$ROOT/tools/elflink.py"

python3 "$LINK" "$BUILD/linker-hello" _start \
    "$BUILD/startup_sysv.o" "$BUILD/runtime2.o" \
    "$BUILD/hello.o" "$BUILD/syscalls_sysv.o"
"$BUILD/linker-hello"
echo "PASS: custom linker -> hello"

python3 "$LINK" "$BUILD/linker-c1-stage1" _start \
    "$BUILD/startup_sysv.o" "$BUILD/runtime1.o" \
    "$BUILD/compiler1.o" "$BUILD/syscalls_sysv.o"
"$BUILD/linker-c1-stage1" < "$ROOT/examples/hello.c" > "$BUILD/linker-c1-hello.o"
python3 "$LINK" "$BUILD/linker-c1-hello" _start \
    "$BUILD/startup_sysv.o" "$BUILD/runtime2.o" \
    "$BUILD/linker-c1-hello.o" "$BUILD/syscalls_sysv.o"
"$BUILD/linker-c1-hello"
echo "PASS: custom linker -> stage1 compiler"
