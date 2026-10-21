#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)
BUILD=${BUILD:-"$ROOT/build"}
BUILD=$(CDPATH= cd -- "$BUILD" && pwd)
LD=${LD:-ld}
LDFLAGS="-static -nostdlib -z noexecstack -e _start"
WORK="$BUILD/env"
mkdir -p "$WORK"
cp "$ROOT/tests/sysv/varargs-int.c" "$WORK/test.c"
cd "$WORK"
C1_INCLUDE="$ROOT/include" C1_STANDARD=c99 "$BUILD/c1-stage2" < test.c > test.o
"$LD" $LDFLAGS -o test \
    "$BUILD/startup_sysv.o" "$BUILD/runtime2.o" \
    test.o "$BUILD/syscalls_sysv.o"
./test
echo "PASS: env/C1_INCLUDE + C1_STANDARD outside repository root"
