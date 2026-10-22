#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)
BUILD=${BUILD:-"$ROOT/build"}
MIN="$BUILD/minimal"
C1_STAGE1=${C1_STAGE1:-"$MIN/c1-stage1.elf"}
WORK="$BUILD/bridge"
UP="$ROOT/windows/upstream/bootstrap-c99"
PE2ELF="$ROOT/tools/pe2elf.py"
TMP=$(mktemp -d "${TMPDIR:-/tmp}/bit-bridge.XXXXXX")
trap 'rm -rf "$TMP"' EXIT
mkdir -p "$WORK" "$TMP/src"
ln -s "$UP" "$TMP/src/bootstrap-c99"
ln -s "$UP/base/include" "$TMP/include"

test -x "$C1_STAGE1" || {
    echo "bridge: C1_STAGE1 is missing: $C1_STAGE1" >&2
    exit 1
}

cd "$TMP"
"$C1_STAGE1" < "$UP/object/compiler.c" > "$TMP/object-c1.exe"
python3 "$PE2ELF" --allow-unresolved --c1-output object "$TMP/object-c1.exe" "$TMP/object-c1-obj.elf"
python3 "$PE2ELF" --allow-unresolved "$TMP/object-c1.exe" "$TMP/object-c1-exe.elf"

"$TMP/object-c1-exe.elf" < "$UP/linker/link.c" > "$TMP/link.exe"
python3 "$PE2ELF" --allow-unresolved "$TMP/link.exe" "$TMP/link.elf"

cd "$ROOT"
"$TMP/object-c1-obj.elf" < "$ROOT/linux/compiler.c" > "$TMP/compiler.c1o"
"$TMP/object-c1-obj.elf" < "$ROOT/windows/bridge/win32_runtime.c" > "$TMP/runtime.c1o"

"$TMP/link.elf" "$TMP/bridge.exe" __c1_start "$TMP/compiler.c1o" "$TMP/runtime.c1o" \
    --import __c1_win_ExitProcess KERNEL32.dll ExitProcess \
    --import __c1_win_ReadFile KERNEL32.dll ReadFile \
    --import __c1_win_WriteFile KERNEL32.dll WriteFile \
    --import __c1_win_CreateFileA KERNEL32.dll CreateFileA \
    --import __c1_win_CloseHandle KERNEL32.dll CloseHandle \
    --import __c1_win_GetEnvironmentVariableA KERNEL32.dll GetEnvironmentVariableA \
    --import __c1_win_GetLocalTime KERNEL32.dll GetLocalTime \
    --import __c1_win_GetProcessHeap KERNEL32.dll GetProcessHeap \
    --import __c1_win_HeapAlloc KERNEL32.dll HeapAlloc \
    --import __c1_win_HeapFree KERNEL32.dll HeapFree

python3 "$PE2ELF" "$TMP/bridge.exe" "$TMP/bridge.elf"
"$TMP/bridge.elf" < "$ROOT/linux/compiler.c" > "$TMP/compiler-bridge.o"

"$BUILD/c1link" "$WORK/c1-stage1" _start \
    "$BUILD/startup_sysv.o" "$BUILD/runtime1.o" \
    "$TMP/compiler-bridge.o" "$BUILD/syscalls_sysv.o"
"$WORK/c1-stage1" < "$ROOT/linux/compiler.c" > "$TMP/compiler-stage2.o"
"$BUILD/c1link" "$WORK/c1-stage2" _start \
    "$BUILD/startup_sysv.o" "$BUILD/runtime1.o" \
    "$TMP/compiler-stage2.o" "$BUILD/syscalls_sysv.o"
cmp "$WORK/c1-stage1" "$WORK/c1-stage2"

"$WORK/c1-stage1" < "$ROOT/examples/hello.c" > "$TMP/hello.o"
"$BUILD/c1link" "$WORK/hello" _start \
    "$BUILD/startup_sysv.o" "$BUILD/runtime2.o" \
    "$TMP/hello.o" "$BUILD/syscalls_sysv.o"
"$WORK/hello"

cp "$TMP/bridge.exe" "$WORK/bridge.exe"
echo "PASS: upstream C1 bridge -> SysV stage1 == stage2"
