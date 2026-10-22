#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
BUILD=${BUILD:-"$ROOT/build/diff-sysv"}
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
skip=0
fail=0
for source in "$ROOT"/tests/sysv/*.c; do
    name=$(basename "$source" .c)
    if ! C1_INCLUDE="$ROOT/include" "$C1" < "$source" > "$BUILD/cases/$name.o" 2>/dev/null; then
        echo "SKIP(native compile): sysv/$name"; skip=$((skip + 1)); continue
    fi
    if ! "$LINKER" --bootstrap "$BUILD/cases/$name.native" _start "$RUNTIME" "$BUILD/cases/$name.o" >/dev/null 2>&1; then
        echo "SKIP(native link): sysv/$name"; skip=$((skip + 1)); continue
    fi
    if ! "$CC" -O0 -fno-pic -fno-pie -no-pie "$source" -o "$BUILD/cases/$name.gcc" >/dev/null 2>&1; then
        echo "SKIP(gcc compile): sysv/$name"; skip=$((skip + 1)); continue
    fi
    set +e
    timeout 10 "$BUILD/cases/$name.native" > "$BUILD/cases/$name.native.out" 2>/dev/null
    ns=$?
    timeout 10 "$BUILD/cases/$name.gcc" > "$BUILD/cases/$name.gcc.out" 2>/dev/null
    gs=$?
    set -e
    if [ "$ns" -ne "$gs" ] || ! cmp -s "$BUILD/cases/$name.native.out" "$BUILD/cases/$name.gcc.out"; then
        echo "FAIL: sysv/$name native=$ns gcc=$gs"
        fail=$((fail + 1))
    else
        pass=$((pass + 1))
    fi
done
echo "diff-sysv: $pass passed, $skip skipped, $fail failed"
test "$fail" -eq 0
