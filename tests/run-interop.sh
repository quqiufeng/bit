#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)
BUILD=${BUILD:-"$ROOT/build"}
CC=${CC:-gcc}
LD=${LD:-ld}
GCCFLAGS="-O2 -ffreestanding -fno-builtin -fno-pic -fno-pie -fno-stack-protector -fno-asynchronous-unwind-tables"
LDFLAGS="-static -nostdlib -z noexecstack -e _start"

"$CC" $GCCFLAGS -c "$ROOT/tests/interop/gcc-helper.c" -o "$BUILD/interop-gcc-helper.o"
"$BUILD/c1-stage2" < "$ROOT/tests/interop/c1-main.c" > "$BUILD/interop-c1-main.o"
"$LD" $LDFLAGS -o "$BUILD/interop-c1-main" \
    "$BUILD/startup_sysv.o" "$BUILD/runtime2.o" \
    "$BUILD/interop-c1-main.o" "$BUILD/interop-gcc-helper.o" "$BUILD/syscalls_sysv.o"
"$BUILD/interop-c1-main"
echo "PASS: interop C1 main -> gcc structs"

"$BUILD/c1-stage2" < "$ROOT/tests/interop/c1-impl.c" > "$BUILD/interop-c1-impl.o"
"$CC" $GCCFLAGS -c "$ROOT/tests/interop/gcc-main.c" -o "$BUILD/interop-gcc-main.o"
"$ROOT/linux/link-hosted.sh" "$BUILD/interop-gcc-main" \
    "$BUILD/interop-gcc-main.o" "$BUILD/interop-c1-impl.o"
"$BUILD/interop-gcc-main"
echo "PASS: interop gcc main -> C1 structs"
