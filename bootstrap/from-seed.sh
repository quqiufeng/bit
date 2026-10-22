#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)
BUILD=${BUILD:-"$ROOT/build"}
mkdir -p "$BUILD"
(cd "$ROOT" && sha256sum -c bootstrap/seed.sha256)
python3 - "$ROOT/bootstrap" "$BUILD" <<'PY'
import binascii, pathlib, sys, os
source = pathlib.Path(sys.argv[1])
build = pathlib.Path(sys.argv[2])
artifacts = {
    'seed-c1': 'seed-c1.hex',
    'seed-link': 'seed-link.hex',
    'seed-startup.o': 'seed-startup.hex',
    'seed-syscalls.o': 'seed-syscalls.hex',
}
for name, hexname in artifacts.items():
    data = binascii.unhexlify(''.join((source / hexname).read_text().split()))
    path = build / name
    path.write_bytes(data)
    os.chmod(path, 0o755)
PY
C1="$BUILD/seed-c1"
LINK="$BUILD/seed-link"
START="$BUILD/seed-startup.o"
SYSCALL="$BUILD/seed-syscalls.o"

"$C1" < "$ROOT/linux/runtime.c" > "$BUILD/seed-runtime1.o"
"$C1" < "$ROOT/linux/compiler.c" > "$BUILD/seed-compiler1.o"
"$LINK" "$BUILD/seed-stage1" _start \
    "$START" "$BUILD/seed-runtime1.o" "$BUILD/seed-compiler1.o" "$SYSCALL"
"$BUILD/seed-stage1" < "$ROOT/linux/runtime.c" > "$BUILD/seed-runtime2.o"
"$BUILD/seed-stage1" < "$ROOT/linux/compiler.c" > "$BUILD/seed-compiler2.o"
cmp "$BUILD/seed-runtime1.o" "$BUILD/seed-runtime2.o"
cmp "$BUILD/seed-compiler1.o" "$BUILD/seed-compiler2.o"
"$LINK" "$BUILD/seed-stage2" _start \
    "$START" "$BUILD/seed-runtime2.o" "$BUILD/seed-compiler2.o" "$SYSCALL"
cmp "$BUILD/seed-stage1" "$BUILD/seed-stage2"
cmp "$C1" "$BUILD/seed-stage2"
"$BUILD/seed-stage2" < "$ROOT/examples/hello.c" > "$BUILD/seed-hello.o"
"$LINK" "$BUILD/seed-hello" _start \
    "$START" "$BUILD/seed-runtime2.o" "$BUILD/seed-hello.o" "$SYSCALL"
"$BUILD/seed-hello"
echo "PASS: build from committed seed bundle (no gcc, no system ld)"
