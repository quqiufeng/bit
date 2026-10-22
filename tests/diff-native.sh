#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
BUILD=${BUILD:-"$ROOT/build/diff"}
OUT="$BUILD/native-c1"
if [ ! -x "$OUT/c1.elf" ]; then
    BUILD="$BUILD" "$ROOT/build-zero.sh" >/dev/null
fi
C1="$OUT/c1.elf"
LINKER="$OUT/native-linker"
RUNTIME="$OUT/runtime.o"
CC=${CC:-gcc}
mkdir -p "$BUILD/diff"
pass=0
fail=0
for source in "$ROOT"/tests/diff/*.c; do
    name=$(basename "$source" .c)
    C1_INCLUDE="$ROOT/include" "$C1" < "$source" > "$BUILD/diff/$name.o"
    "$LINKER" --bootstrap "$BUILD/diff/$name.native" _start "$RUNTIME" "$BUILD/diff/$name.o"
    "$CC" -O0 -fno-pic -fno-pie -no-pie "$source" -o "$BUILD/diff/$name.gcc"
    set +e
    timeout 10 "$BUILD/diff/$name.native" > "$BUILD/diff/$name.native.out" 2> "$BUILD/diff/$name.native.err"
    nstatus=$?
    timeout 10 "$BUILD/diff/$name.gcc" > "$BUILD/diff/$name.gcc.out" 2> "$BUILD/diff/$name.gcc.err"
    gstatus=$?
    set -e
    if [ "$nstatus" -ne "$gstatus" ]; then
        echo "FAIL: $name exit native=$nstatus gcc=$gstatus"
        fail=$((fail + 1))
    elif ! cmp -s "$BUILD/diff/$name.native.out" "$BUILD/diff/$name.gcc.out"; then
        echo "FAIL: $name stdout differs"
        fail=$((fail + 1))
    else
        echo "PASS: diff/$name"
        pass=$((pass + 1))
    fi
done
echo "diff-native: $pass passed, $fail failed"
test "$fail" -eq 0
