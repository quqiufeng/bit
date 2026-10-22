#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
BUILD=${BUILD:-"$ROOT/build/fuzz-hosted-abi"}
OUT="$BUILD/native-c1"
if [ ! -x "$OUT/c1.elf" ]; then
    BUILD="$BUILD" "$ROOT/build-zero.sh" >/dev/null
fi
C1="$OUT/c1.elf"
CC=${CC:-gcc}
mkdir -p "$BUILD/cases"
"$CC" -c "$ROOT/tests/hosted-abi-trampoline.S" -o "$BUILD/cases/trampoline.o"
"$CC" -O0 -fno-pic -fno-pie -c "$ROOT/tests/hosted-abi-control.c" -o "$BUILD/cases/control.o"
"$C1" < "$ROOT/tests/hosted-abi/c1-target.c" > "$BUILD/cases/c1-target.o"
"$C1" < "$ROOT/tests/hosted-abi/main.c" > "$BUILD/cases/c1-main.o"
"$CC" -O0 -fno-pic -fno-pie -c "$ROOT/tests/hosted-abi/main.c" -o "$BUILD/cases/gcc-main.o"
"$ROOT/linux/link-hosted.sh" "$BUILD/cases/c1-main" \
    "$BUILD/cases/c1-main.o" "$BUILD/cases/c1-target.o" \
    "$BUILD/cases/control.o" "$BUILD/cases/trampoline.o"
"$CC" -O0 -fno-pic -fno-pie -no-pie -o "$BUILD/cases/gcc-main" \
    "$BUILD/cases/gcc-main.o" "$BUILD/cases/c1-target.o" \
    "$BUILD/cases/control.o" "$BUILD/cases/trampoline.o"
set +e
"$BUILD/cases/c1-main" > "$BUILD/cases/c1.out" 2> "$BUILD/cases/c1.err"
cs=$?
"$BUILD/cases/gcc-main" > "$BUILD/cases/gcc.out" 2> "$BUILD/cases/gcc.err"
gs=$?
set -e
if [ "$cs" -ne "$gs" ] || ! cmp -s "$BUILD/cases/c1.out" "$BUILD/cases/gcc.out"; then
    echo "FAIL: hosted ABI native=$cs gcc=$gs"
    exit 1
fi
echo "PASS: hosted ABI cross-boundary pollution native=$cs gcc=$gs"
