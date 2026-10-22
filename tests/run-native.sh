#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
BUILD=${BUILD:-"$ROOT/build/native-run"}
OUT="$BUILD/native-c1"
if [ ! -x "$OUT/c1.elf" ]; then
    BUILD="$BUILD" "$ROOT/build-zero.sh" >/dev/null
fi
C1="$OUT/c1.elf"
LINKER="$OUT/native-linker"
RUNTIME="$OUT/runtime.o"
mkdir -p "$BUILD/native-sysv"
for source in "$ROOT"/tests/sysv/*.c; do
    name=$(basename "$source" .c)
    C1_INCLUDE="$ROOT/include" "$C1" < "$source" > "$BUILD/native-sysv/$name.o"
    "$LINKER" --bootstrap "$BUILD/native-sysv/$name" _start "$RUNTIME" "$BUILD/native-sysv/$name.o" >/dev/null
    "$BUILD/native-sysv/$name"
    echo "PASS: native-sysv/$name"
done
echo "PASS: Native C1 + native linker passed the SysV corpus"
