#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../../.." && pwd)
BUILD=${BUILD:-"$ROOT/build"}
OUT="$BUILD/c0v2-from-a1"
mkdir -p "$OUT"
"$ROOT/bootstrap/linux/a1/a1-selfhost.sh" >/dev/null
"$BUILD/a1-selfhost/stage1.elf" < "$ROOT/bootstrap/linux/c0/c0v2.a1" > "$OUT/c0v2.elf"
chmod +x "$OUT/c0v2.elf"
printf 'int main(void){return 42;}\n' | "$OUT/c0v2.elf" > "$OUT/tiny.a1"
"$BUILD/a1/a1" < "$OUT/tiny.a1" > "$OUT/tiny.elf"
chmod +x "$OUT/tiny.elf"
"$OUT/tiny.elf"
test $? -eq 42
echo "PASS: A1 assembles frozen C0v2 A1 source"
