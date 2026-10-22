#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../../.." && pwd)
BUILD=${BUILD:-"$ROOT/build"}
BASE="$ROOT/windows/upstream/bootstrap-c99/base"
WORK="$BUILD/c0/chain"
C0="$BUILD/c0/c0"
A1="$BUILD/a1/a1"
PE2ELF="$ROOT/tools/pe2elf.py"
mkdir -p "$WORK"

"$C0" < "$BASE/c1/bootstrap.c" > "$WORK/c1-bridge.a1"
"$A1" < "$WORK/c1-bridge.a1" > "$WORK/c1-bridge.elf"
chmod +x "$WORK/c1-bridge.elf"

"$WORK/c1-bridge.elf" < "$BASE/c1/bootstrap-capacity.c" > "$WORK/c1-capacity.exe"
python3 "$PE2ELF" --allow-unresolved "$WORK/c1-capacity.exe" "$WORK/c1-capacity.elf"
chmod +x "$WORK/c1-capacity.elf"

"$WORK/c1-capacity.elf" < "$BASE/c1/compiler.c" > "$WORK/c1-stage0.exe"
python3 "$PE2ELF" --allow-unresolved "$WORK/c1-stage0.exe" "$WORK/c1-stage0.elf"
chmod +x "$WORK/c1-stage0.elf"

"$WORK/c1-stage0.elf" < "$BASE/c1/compiler.c" > "$WORK/c1-stage1.exe"
python3 "$PE2ELF" --allow-unresolved "$WORK/c1-stage1.exe" "$WORK/c1-stage1.elf"
chmod +x "$WORK/c1-stage1.elf"

"$WORK/c1-stage1.elf" < "$BASE/c1/compiler.c" > "$WORK/c1-stage2.exe"
cmp "$WORK/c1-stage1.exe" "$WORK/c1-stage2.exe"

if [ -f "$BUILD/minimal/c1-stage0.exe" ]; then
    cmp "$WORK/c1-stage0.exe" "$BUILD/minimal/c1-stage0.exe"
    cmp "$WORK/c1-stage1.exe" "$BUILD/minimal/c1-stage1.exe"
    cmp "$WORK/c1-stage2.exe" "$BUILD/minimal/c1-stage2.exe"
fi
echo "PASS: C0 -> bootstrap -> capacity -> C1 stage1 == stage2"
