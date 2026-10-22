#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)
BASE="$ROOT/windows/upstream/bootstrap-c99/base"
OUT="$ROOT/build/minimal"
PE2ELF="$ROOT/tools/pe2elf.py"
mkdir -p "$OUT"

sed 's/#.*//' "$BASE/seed.hex" | "$ROOT/bootstrap/linux/seed.elf" > "$OUT/a0-seed.exe"

run_pe() {
    pe=$1
    input=$2
    output=$3
    elf=${pe%.exe}.elf
    python3 "$PE2ELF" --allow-unresolved "$pe" "$elf"
    "$elf" < "$input" > "$output"
}

cd "$BASE"
run_pe "$OUT/a0-seed.exe" a0.a0 "$OUT/a0-stage1.exe"
run_pe "$OUT/a0-stage1.exe" a0.a0 "$OUT/a0-stage2.exe"
cmp "$OUT/a0-stage1.exe" "$OUT/a0-stage2.exe"

run_pe "$OUT/a0-stage2.exe" a1.a0 "$OUT/a1-stage0.exe"
run_pe "$OUT/a1-stage0.exe" a1.a0 "$OUT/a1-stage1.exe"
run_pe "$OUT/a1-stage1.exe" a1.a0 "$OUT/a1-stage2.exe"
cmp "$OUT/a1-stage1.exe" "$OUT/a1-stage2.exe"

run_pe "$OUT/a1-stage2.exe" c0.a1 "$OUT/c0.exe"
run_pe "$OUT/c0.exe" c1/bootstrap.c "$OUT/c1-bridge.a1"
run_pe "$OUT/a1-stage2.exe" "$OUT/c1-bridge.a1" "$OUT/c1-bridge.exe"
run_pe "$OUT/c1-bridge.exe" c1/bootstrap-capacity.c "$OUT/c1-capacity.exe"
run_pe "$OUT/c1-capacity.exe" c1/compiler.c "$OUT/c1-stage0.exe"
run_pe "$OUT/c1-stage0.exe" c1/compiler.c "$OUT/c1-stage1.exe"
run_pe "$OUT/c1-stage1.exe" c1/compiler.c "$OUT/c1-stage2.exe"
cmp "$OUT/c1-stage1.exe" "$OUT/c1-stage2.exe"
if python3 "$PE2ELF" "$OUT/c1-stage1.exe" "$OUT/strict-should-fail.elf" 2>"$OUT/strict-imports.log"; then
    echo "FAIL: strict pe2elf accepted unresolved imports" >&2
    exit 1
fi
grep -q "unresolved import KERNEL32.dll!HeapReAlloc" "$OUT/strict-imports.log"
echo "PASS: strict pe2elf rejects unresolved imports with symbol names"
echo "PASS: minimal seed -> A0 -> A1 -> C0 -> C1 (upstream Windows C1) on Linux"
