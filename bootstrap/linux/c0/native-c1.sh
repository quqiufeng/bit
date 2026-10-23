#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../../.." && pwd)
BUILD=${BUILD:-"$ROOT/build"}
OUT="$BUILD/native-c1"
mkdir -p "$OUT"
A0="$OUT/a0.seed.elf"
"$ROOT/bootstrap/linux/a0/from-hex.sh" "$A0"
A1="$OUT/a1.seed.elf"
"$ROOT/bootstrap/linux/a1/from-hex.sh" "$A1"
C0V2="$OUT/c0v2-seed.elf"
"$A1" < "$ROOT/bootstrap/linux/c0/c0v2.a1" > "$C0V2"
chmod +x "$C0V2"
"$A0" < "$ROOT/bootstrap/linux/a0/a0.seed.hex" > "$OUT/a0-regen.seed.elf"
cmp "$A0" "$OUT/a0-regen.seed.elf"
"$A1" < "$ROOT/bootstrap/linux/a1/a1.seed.hex" > "$OUT/a1-regen.seed.elf"
cmp "$A1" "$OUT/a1-regen.seed.elf"
"$C0V2" < "$ROOT/bootstrap/linux/c0/c0v2.c" > "$OUT/c0v2-regen.a1"
cmp "$OUT/c0v2-regen.a1" "$ROOT/bootstrap/linux/c0/c0v2.a1"
cat "$ROOT/linux/standalone-runtime.c" "$ROOT/linux/standalone-link.c" > "$OUT/linker-source.c"
"$C0V2" < "$OUT/linker-source.c" > "$OUT/linker.a1"
"$ROOT/bootstrap/linux/c0/native/insert-a1.sh" "$OUT/linker.a1" \
    "$ROOT/bootstrap/linux/c0/native/linker-helpers.a1" "$OUT/linker-full.a1"
"$A1" < "$OUT/linker-full.a1" > "$OUT/c1l.elf"
chmod +x "$OUT/c1l.elf"
"$C0V2" < "$ROOT/bootstrap/linux/c0/native/compiler-bootstrap.c" > "$OUT/bootstrap.a1"
"$ROOT/bootstrap/linux/c0/native/make.sh" "$OUT/bootstrap.a1" "$ROOT/bootstrap/linux/c0/native/helpers.a1" "$OUT/bootstrap-full.a1"
"$A1" < "$OUT/bootstrap-full.a1" > "$OUT/c1-bootstrap.elf"
chmod +x "$OUT/c1-bootstrap.elf"
cd "$ROOT"
C1_INCLUDE="$ROOT" "$OUT/c1-bootstrap.elf" < linux/compiler.c > "$OUT/compiler.o"
C1_INCLUDE="$ROOT" "$OUT/c1-bootstrap.elf" < linux/runtime.c > "$OUT/runtime.o"
{
    printf '%s\n' "$OUT/c1.elf"
    printf '%s\n' _start
    printf '%s\n' "$OUT/runtime.o"
    printf '%s\n' "$OUT/compiler.o"
} | "$OUT/c1l.elf" >/dev/null
chmod +x "$OUT/c1.elf"
cd "$ROOT"
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
link_one() {
    out=$1
    entry=$2
    shift 2
    {
        printf '%s\n' "$out"
        printf '%s\n' "$entry"
        for f in "$@"; do
            printf '%s\n' "$f"
        done
    } | "$OUT/c1l.elf" >/dev/null
}
link_one "$OUT/native-linker" _start "$OUT/runtime.o" "$OUT/linker.o"
"$OUT/native-linker" --bootstrap "$OUT/native-linker2" _start \
    "$OUT/runtime.o" "$OUT/linker.o"
cmp "$OUT/native-linker" "$OUT/native-linker2"
link_one "$OUT/stage1" _start "$OUT/runtime.o" "$OUT/compiler.o"
"$OUT/stage1" < "$OUT/tiny.c" > "$OUT/tiny-stage1.o"
cmp "$OUT/tiny.o" "$OUT/tiny-stage1.o"
C1_INCLUDE="$ROOT" "$OUT/stage1" < linux/compiler.c > "$OUT/compiler-stage2.o"
cmp "$OUT/compiler.o" "$OUT/compiler-stage2.o"
link_one "$OUT/stage2" _start "$OUT/runtime.o" "$OUT/compiler-stage2.o"
cmp "$OUT/stage1" "$OUT/stage2"
echo "PASS: native Linux C1 and native linker reach stage1 == stage2"
