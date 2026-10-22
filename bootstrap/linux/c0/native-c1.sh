#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../../.." && pwd)
BUILD=${BUILD:-"$ROOT/build"}
OUT="$BUILD/native-c1"
mkdir -p "$OUT"
"$BUILD/c0v2/c0v2" < "$ROOT/bootstrap/linux/c0/native/compiler-runtime.c" > "$OUT/compiler.a1"
python3 "$ROOT/bootstrap/linux/c0/native/make.py" "$OUT/compiler.a1" "$ROOT/bootstrap/linux/c0/native/helpers.a1" "$OUT/compiler-full.a1"
"$BUILD/a1/a1" < "$OUT/compiler-full.a1" > "$OUT/c1.elf"
chmod +x "$OUT/c1.elf"
printf 'int main(void){return 42;}\n' > "$OUT/tiny.c"
"$OUT/c1.elf" < "$OUT/tiny.c" > "$OUT/tiny.o"
head -c 4 "$OUT/tiny.o" | od -An -tx1 | grep -q '7f 45 4c 46'
echo "PASS: native Linux C1 compiles an ELF object"
