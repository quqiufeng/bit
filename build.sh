#!/bin/sh
set -eu
cd "$(dirname "$0")"

CC=${CC:-gcc}
LD=${LD:-ld}
BUILD=build
CFLAGS="-std=gnu89 -O2 -ffreestanding -fno-builtin -fno-pic -fno-pie -fno-stack-protector -fno-asynchronous-unwind-tables -I."
LDFLAGS="-static -nostdlib -z noexecstack -e _start"

mkdir -p "$BUILD"

"$CC" $CFLAGS -c linux/compiler.c -o "$BUILD/compiler_gcc.o"
"$CC" $CFLAGS -DC1_SYSV_HOST -c linux/runtime.c -o "$BUILD/runtime_sysv.o"
"$CC" -c linux/syscalls-sysv.S -o "$BUILD/syscalls_sysv.o"
"$CC" -c linux/startup-sysv.S -o "$BUILD/startup_sysv.o"
"$CC" -c linux/syscalls-win64.S -o "$BUILD/syscalls_win64.o"
"$CC" -c linux/startup-win64.S -o "$BUILD/startup_win64.o"

"$LD" $LDFLAGS -o "$BUILD/c1-stage0" \
    "$BUILD/startup_sysv.o" "$BUILD/compiler_gcc.o" \
    "$BUILD/runtime_sysv.o" "$BUILD/syscalls_sysv.o"

"$BUILD/c1-stage0" < linux/runtime.c > "$BUILD/runtime1.o"
"$BUILD/c1-stage0" < linux/compiler.c > "$BUILD/compiler1.o"
"$LD" $LDFLAGS -o "$BUILD/c1-stage1" \
    "$BUILD/startup_win64.o" "$BUILD/runtime1.o" \
    "$BUILD/compiler1.o" "$BUILD/syscalls_win64.o"

"$BUILD/c1-stage1" < linux/runtime.c > "$BUILD/runtime2.o"
"$BUILD/c1-stage1" < linux/compiler.c > "$BUILD/compiler2.o"
cmp "$BUILD/runtime1.o" "$BUILD/runtime2.o"
cmp "$BUILD/compiler1.o" "$BUILD/compiler2.o"

"$LD" $LDFLAGS -o "$BUILD/c1-stage2" \
    "$BUILD/startup_win64.o" "$BUILD/runtime2.o" \
    "$BUILD/compiler2.o" "$BUILD/syscalls_win64.o"
cmp "$BUILD/c1-stage1" "$BUILD/c1-stage2"

"$BUILD/c1-stage2" < examples/hello.c > "$BUILD/hello.o"
"$LD" $LDFLAGS -o "$BUILD/hello" \
    "$BUILD/startup_win64.o" "$BUILD/runtime2.o" \
    "$BUILD/hello.o" "$BUILD/syscalls_win64.o"
"$BUILD/hello"

echo "PASS: stage1 = stage2; hello"
