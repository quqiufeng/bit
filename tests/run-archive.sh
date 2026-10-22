#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)
BUILD=${BUILD:-"$ROOT/build"}
C1=${C1:-$C1}
$C1 < "$ROOT/tests/linker/answer.c" > "$BUILD/archive-answer.o"
$C1 < "$ROOT/tests/linker/unused.c" > "$BUILD/archive-unused.o"
$C1 < "$ROOT/tests/linker/main.c" > "$BUILD/archive-main.o"
rm -f "$BUILD/libarchive-test.a"
ar rcs "$BUILD/libarchive-test.a" "$BUILD/archive-answer.o" "$BUILD/archive-unused.o"
"$BUILD/c1link" "$BUILD/archive-test" _start \
    "$BUILD/startup_sysv.o" "$BUILD/runtime2.o" \
    "$BUILD/archive-main.o" "$BUILD/libarchive-test.a" "$BUILD/syscalls_sysv.o"
"$BUILD/archive-test"
echo "PASS: c1link archive selective member"
