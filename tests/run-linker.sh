#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)
BUILD=${BUILD:-"$ROOT/build"}
LINK="$BUILD/c1link"

"$LINK" "$BUILD/linker-hello" _start \
    "$BUILD/startup_sysv.o" "$BUILD/runtime2.o" \
    "$BUILD/hello.o" "$BUILD/syscalls_sysv.o"
"$BUILD/linker-hello"
echo "PASS: custom linker -> hello"

"$LINK" "$BUILD/linker-c1-stage1" _start \
    "$BUILD/startup_sysv.o" "$BUILD/runtime1.o" \
    "$BUILD/compiler1.o" "$BUILD/syscalls_sysv.o"
"$BUILD/linker-c1-stage1" < "$ROOT/examples/hello.c" > "$BUILD/linker-c1-hello.o"
"$BUILD/c1link" "$BUILD/c-linker-hello" _start \
    "$BUILD/startup_sysv.o" "$BUILD/runtime2.o" \
    "$BUILD/hello.o" "$BUILD/syscalls_sysv.o"
"$BUILD/c-linker-hello"
if command -v readelf >/dev/null 2>&1; then
    readelf -S "$BUILD/c-linker-hello" | grep -q '.symtab'
    readelf -s "$BUILD/c-linker-hello" | grep -q ' main'
fi
echo "PASS: C linker -> hello"

"$BUILD/c1link" "$BUILD/c-linker-c1-stage1" _start \
    "$BUILD/startup_sysv.o" "$BUILD/runtime1.o" \
    "$BUILD/compiler1.o" "$BUILD/syscalls_sysv.o"
"$BUILD/c-linker-c1-stage1" < "$ROOT/examples/hello.c" > "$BUILD/c-linker-c1-hello.o"
"$BUILD/c1link" "$BUILD/c-linker-c1-hello" _start \
    "$BUILD/startup_sysv.o" "$BUILD/runtime2.o" \
    "$BUILD/c-linker-c1-hello.o" "$BUILD/syscalls_sysv.o"
"$BUILD/c-linker-c1-hello"
echo "PASS: C linker -> stage1 compiler"

"$LINK" "$BUILD/linker-c1-hello" _start \
    "$BUILD/startup_sysv.o" "$BUILD/runtime2.o" \
    "$BUILD/linker-c1-hello.o" "$BUILD/syscalls_sysv.o"
"$BUILD/linker-c1-hello"
echo "PASS: custom linker -> stage1 compiler"
