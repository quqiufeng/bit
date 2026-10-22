#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../../.." && pwd)
BUILD=${BUILD:-"$ROOT/build"}
OUT="$BUILD/c0v2-from-a1-selfhost"
mkdir -p "$OUT"
"$ROOT/bootstrap/linux/c0/c0v2-from-a1.sh" >/dev/null
"$BUILD/c0v2-from-a1/c0v2.elf" < "$ROOT/bootstrap/linux/c0/c0v2.c" > "$OUT/stage2.a1"
"$ROOT/bootstrap/linux/a1/a1.seed.elf" < "$OUT/stage2.a1" > "$OUT/stage2.elf"
cmp "$BUILD/c0v2-from-a1/c0v2.elf" "$OUT/stage2.elf"
echo "PASS: C0v2 from A1 is a binary fixed point"
