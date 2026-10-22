#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
BUILD=${BUILD:-"$ROOT/build/zero"}
BUILD="$BUILD" "$ROOT/bootstrap/native-zero.sh"
OUT="$BUILD/native-c1"
"$OUT/stage1" < "$ROOT/examples/hello.c" > "$OUT/hello.o"
"$OUT/native-linker" --bootstrap "$OUT/hello" _start "$OUT/runtime.o" "$OUT/hello.o"
{
    printf '%s\n' "$OUT/hello-standalone"
    printf '%s\n' _start
    printf '%s\n' "$OUT/runtime.o"
    printf '%s\n' "$OUT/hello.o"
} | "$OUT/c1l.elf" >/dev/null
cmp "$OUT/hello" "$OUT/hello-standalone"
"$OUT/hello-standalone"
echo "PASS: zero-dependency build produced native C1, standalone link, and ran hello"
