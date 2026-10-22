#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
BUILD=${BUILD:-"$ROOT/build/zero"}
BUILD="$BUILD" "$ROOT/bootstrap/native-zero.sh"
OUT="$BUILD/native-c1"
"$OUT/stage1" < "$ROOT/examples/hello.c" > "$OUT/hello.o"
"$OUT/native-linker" --bootstrap "$OUT/hello" _start "$OUT/runtime.o" "$OUT/hello.o"
"$OUT/hello"
echo "PASS: zero-dependency build produced native C1 and ran hello"
