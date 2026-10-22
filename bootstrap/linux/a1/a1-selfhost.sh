#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
BUILD=${BUILD:-"$ROOT/build"}
OUT="$BUILD/a1-selfhost"
mkdir -p "$OUT"
"$ROOT/bootstrap/linux/c0/c0v2-bss-build.sh" >/dev/null
"$BUILD/c0v2-bss/c0v2-bss" < "$ROOT/bootstrap/linux/a1/a1_self.c" > "$OUT/stage1.a1"
"$BUILD/a1/a1" < "$OUT/stage1.a1" > "$OUT/stage1.elf"
chmod +x "$OUT/stage1.elf"
"$OUT/stage1.elf" < "$OUT/stage1.a1" > "$OUT/stage2.elf"
cmp "$OUT/stage1.elf" "$OUT/stage2.elf"
echo "PASS: A1 source-level fixed point (binary)"
