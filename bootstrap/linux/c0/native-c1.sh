#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../../.." && pwd)
BUILD=${BUILD:-"$ROOT/build"}
OUT="$BUILD/native-c1"
mkdir -p "$OUT"
C0V2="$BUILD/c0v2-from-a1/c0v2.elf"
[ -x "$C0V2" ] || C0V2="$BUILD/c0v2/c0v2"
"$C0V2" < "$ROOT/bootstrap/linux/c0/native/compiler-runtime.c" > "$OUT/compiler.a1"
python3 "$ROOT/bootstrap/linux/c0/native/make.py" "$OUT/compiler.a1" "$ROOT/bootstrap/linux/c0/native/helpers.a1" "$OUT/compiler-full.a1"
"$BUILD/a1/a1" < "$OUT/compiler-full.a1" > "$OUT/c1.elf"
chmod +x "$OUT/c1.elf"
printf 'int main(void){return 42;}\n' > "$OUT/tiny.c"
"$OUT/c1.elf" < "$OUT/tiny.c" > "$OUT/tiny.o"
head -c 4 "$OUT/tiny.o" | od -An -tx1 | grep -q '7f 45 4c 46'
cd "$ROOT"
C1_INCLUDE="$ROOT" "$OUT/c1.elf" < linux/compiler.c > "$OUT/compiler.o"
C1_INCLUDE="$ROOT" "$OUT/c1.elf" < linux/runtime.c > "$OUT/runtime.o"
"$BUILD/c1link" "$OUT/stage1" _start \
    "$BUILD/startup_sysv.o" "$OUT/runtime.o" "$OUT/compiler.o" "$BUILD/syscalls_sysv.o"
"$OUT/stage1" < "$OUT/tiny.c" > "$OUT/tiny-stage1.o"
cmp "$OUT/tiny.o" "$OUT/tiny-stage1.o"
C1_INCLUDE="$ROOT" "$OUT/stage1" < linux/compiler.c > "$OUT/compiler-stage2.o"
cmp "$OUT/compiler.o" "$OUT/compiler-stage2.o"
echo "PASS: native Linux C1 compiles an ELF object and reaches stage1 == stage2"
