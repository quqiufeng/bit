#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
BUILD=${BUILD:-"$ROOT/build/fuzz-iso"}
OUT="$BUILD/native-c1"
if [ ! -x "$OUT/c1.elf" ]; then
    BUILD="$BUILD" "$ROOT/build-zero.sh" >/dev/null
fi
C1="$OUT/c1.elf"
LINKER="$OUT/native-linker"
RUNTIME="$OUT/runtime.o"
CC=${CC:-gcc}
mkdir -p "$BUILD/cases"
pass=0
fail=0
for source in "$ROOT"/tests/iso/*.c; do
    name=$(basename "$source" .c)
    C1_INCLUDE="$ROOT/include" "$C1" < "$source" > "$BUILD/cases/$name.o"
    "$LINKER" --bootstrap "$BUILD/cases/$name.native" _start "$RUNTIME" "$BUILD/cases/$name.o" >/dev/null
    "$CC" -O0 -fno-pic -fno-pie -no-pie "$source" -o "$BUILD/cases/$name.gcc"
    set +e
    "$BUILD/cases/$name.native"; ns=$?
    "$BUILD/cases/$name.gcc"; gs=$?
    set -e
    if [ "$ns" -ne "$gs" ]; then
        echo "FAIL: iso/$name native=$ns gcc=$gs"
        fail=$((fail + 1))
    else
        echo "PASS: iso/$name"
        pass=$((pass + 1))
    fi
done
echo "fuzz-iso: $pass passed, $fail failed"
test "$fail" -eq 0
