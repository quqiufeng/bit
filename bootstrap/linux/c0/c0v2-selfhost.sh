#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../../.." && pwd)
BUILD=${BUILD:-"$ROOT/build"}
OUT="$BUILD/c0v2-selfhost"
mkdir -p "$OUT"
"$ROOT/bootstrap/linux/c0/c0v2-build.sh" >/dev/null
"$BUILD/c0v2/c0v2" < "$ROOT/bootstrap/linux/c0/c0v2.c" > "$OUT/stage1.a1"
"$BUILD/a1/a1" < "$OUT/stage1.a1" > "$OUT/stage1.elf"
chmod +x "$OUT/stage1.elf"
"$OUT/stage1.elf" < "$ROOT/bootstrap/linux/c0/c0v2.c" > "$OUT/stage2.a1"
"$BUILD/a1/a1" < "$OUT/stage2.a1" > "$OUT/stage2.elf"
cmp "$OUT/stage1.elf" "$OUT/stage2.elf"
echo "PASS: C0v2 source-level fixed point (binary)"
