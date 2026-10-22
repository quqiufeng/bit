#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../../.." && pwd)
BUILD=${BUILD:-"$ROOT/build"}
OUT="$BUILD/c0v2"
mkdir -p "$OUT"
"$BUILD/c1-stage1" < "$ROOT/bootstrap/linux/c0/c0v2.c" > "$OUT/c0v2.o"
"$BUILD/c1link" "$OUT/c0v2" _start \
    "$BUILD/startup_sysv.o" "$BUILD/runtime2.o" \
    "$OUT/c0v2.o" "$BUILD/syscalls_sysv.o"
printf 'int main(void){return 42;}\n' | "$OUT/c0v2" > "$OUT/smoke.a1"
"$BUILD/a1/a1" < "$OUT/smoke.a1" > "$OUT/smoke.elf"
chmod +x "$OUT/smoke.elf"
set +e
"$OUT/smoke.elf"
status=$?
set -e
test "$status" -eq 42
