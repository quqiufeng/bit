#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../../.." && pwd)
BUILD=${BUILD:-"$ROOT/build"}
OUT="$BUILD/a1"
mkdir -p "$OUT"
"$BUILD/c1-stage1" < "$ROOT/bootstrap/linux/a1/a1.c" > "$OUT/a1.o"
"$BUILD/c1link" "$OUT/a1" _start \
    "$BUILD/startup_sysv.o" "$BUILD/runtime2.o" \
    "$OUT/a1.o" "$BUILD/syscalls_sysv.o"
"$OUT/a1" < "$ROOT/bootstrap/linux/a1/smoke.a1" > "$OUT/smoke.elf"
chmod +x "$OUT/smoke.elf"
set +e
"$OUT/smoke.elf"
status=$?
set -e
test "$status" -eq 42
"$ROOT/bootstrap/linux/a1/a1_ref" >/dev/null 2>&1 || true
