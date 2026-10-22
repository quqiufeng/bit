#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../../.." && pwd)
BUILD=${BUILD:-"$ROOT/build"}
OUT="$BUILD/native-c1"
mkdir -p "$OUT"
A0="$OUT/a0.seed.elf"
"$ROOT/bootstrap/linux/seed.elf" < "$ROOT/bootstrap/linux/a0/a0.seed.hex" > "$A0"
chmod +x "$A0"
A1="$OUT/a1.seed.elf"
"$A0" < "$ROOT/bootstrap/linux/a1/a1.seed.hex" > "$A1"
chmod +x "$A1"
LINKER="$OUT/linker.seed.elf"
"$A1" < "$ROOT/bootstrap/linux/c0/native/linker.seed.hex" > "$LINKER"
chmod +x "$LINKER"
C0V2="$OUT/c0v2-seed.elf"
"$A1" < "$ROOT/bootstrap/linux/c0/c0v2.a1" > "$C0V2"
chmod +x "$C0V2"
"$C0V2" < "$ROOT/bootstrap/linux/c0/native/compiler-runtime.c" > "$OUT/compiler.a1"
"$ROOT/bootstrap/linux/c0/native/make.sh" "$OUT/compiler.a1" "$ROOT/bootstrap/linux/c0/native/helpers.a1" "$OUT/compiler-full.a1"
"$A1" < "$OUT/compiler-full.a1" > "$OUT/c1.elf"
chmod +x "$OUT/c1.elf"
printf 'int main(void){return 42;}\n' > "$OUT/tiny.c"
"$OUT/c1.elf" < "$OUT/tiny.c" > "$OUT/tiny.o"
head -c 4 "$OUT/tiny.o" | od -An -tx1 | grep -q '7f 45 4c 46'
cd "$ROOT"
C1_INCLUDE="$ROOT" "$OUT/c1.elf" < linux/compiler.c > "$OUT/compiler.o"
C1_INCLUDE="$ROOT" "$OUT/c1.elf" -E < linux/compiler.c > "$OUT/compiler.i"
C1_INCLUDE="$ROOT" "$OUT/c1.elf" < "$OUT/compiler.i" > "$OUT/compiler-i.o"
cmp "$OUT/compiler.o" "$OUT/compiler-i.o"
C1_INCLUDE="$ROOT" "$OUT/c1.elf" < linux/runtime.c > "$OUT/runtime.o"
C1_INCLUDE="$ROOT" "$OUT/c1.elf" < linux/linker.c > "$OUT/linker.o"
"$LINKER" --bootstrap "$OUT/native-linker" _start \
    "$OUT/runtime.o" "$OUT/linker.o"
"$OUT/native-linker" --bootstrap "$OUT/native-linker2" _start \
    "$OUT/runtime.o" "$OUT/linker.o"
cmp "$OUT/native-linker" "$OUT/native-linker2"
"$OUT/native-linker" --bootstrap "$OUT/stage1" _start \
    "$OUT/runtime.o" "$OUT/compiler.o"
"$OUT/stage1" < "$OUT/tiny.c" > "$OUT/tiny-stage1.o"
cmp "$OUT/tiny.o" "$OUT/tiny-stage1.o"
C1_INCLUDE="$ROOT" "$OUT/stage1" < linux/compiler.c > "$OUT/compiler-stage2.o"
cmp "$OUT/compiler.o" "$OUT/compiler-stage2.o"
"$OUT/native-linker" --bootstrap "$OUT/stage2" _start \
    "$OUT/runtime.o" "$OUT/compiler-stage2.o"
cmp "$OUT/stage1" "$OUT/stage2"
echo "PASS: native Linux C1 and native linker reach stage1 == stage2"
