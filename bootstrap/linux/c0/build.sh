#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../../.." && pwd)
BUILD=${BUILD:-"$ROOT/build"}
A1="$BUILD/a1/a1"
OUT="$BUILD/c0"
mkdir -p "$OUT"
"$BUILD/c1-stage1" < "$ROOT/bootstrap/linux/c0/c0.c" > "$OUT/c0.o"
"$BUILD/c1link" "$OUT/c0" _start \
    "$BUILD/startup_sysv.o" "$BUILD/runtime2.o" \
    "$OUT/c0.o" "$BUILD/syscalls_sysv.o"
printf 'int main(void){return 42;}\n' | "$OUT/c0" > "$OUT/smoke.a1"
"$A1" < "$OUT/smoke.a1" > "$OUT/smoke.elf"
chmod +x "$OUT/smoke.elf"
set +e
"$OUT/smoke.elf"
status=$?
set -e
test "$status" -eq 42

printf 'int g;\nint a[4];\nint main(void){return 42;}\n' | "$OUT/c0" > "$OUT/globals.a1"
"$A1" < "$OUT/globals.a1" > "$OUT/globals.elf"
chmod +x "$OUT/globals.elf"
set +e
"$OUT/globals.elf"
status=$?
set -e
test "$status" -eq 42
