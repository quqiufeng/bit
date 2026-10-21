#!/bin/sh
set -eu
cd "$(dirname "$0")"

CC=${CC:-gcc}
BUILD=build
CFLAGS="-std=gnu89 -O2 -ffreestanding -fno-builtin -fno-pic -fno-pie -fno-stack-protector -fno-asynchronous-unwind-tables -I."

mkdir -p "$BUILD"

"$CC" $CFLAGS -c linux/compiler.c -o "$BUILD/compiler_gcc.o"
"$CC" $CFLAGS -c linux/runtime.c -o "$BUILD/runtime_sysv.o"
"$CC" -c linux/syscalls-sysv.S -o "$BUILD/syscalls_sysv.o"
"$CC" -c linux/startup-sysv.S -o "$BUILD/startup_sysv.o"

python3 tools/elflink.py "$BUILD/c1-stage0" _start \
    "$BUILD/startup_sysv.o" "$BUILD/compiler_gcc.o" \
    "$BUILD/runtime_sysv.o" "$BUILD/syscalls_sysv.o"

# Bootstrap c1link with the Python ELF linker prototype.  Everything after this
# point that produces a C1 executable uses c1link; system ld is only used by
# the hosted glibc tests.
"$BUILD/c1-stage0" < linux/linker.c > "$BUILD/linker0.o"
python3 tools/elflink.py "$BUILD/c1link" _start \
    "$BUILD/startup_sysv.o" "$BUILD/runtime_sysv.o" \
    "$BUILD/linker0.o" "$BUILD/syscalls_sysv.o"

"$BUILD/c1-stage0" < linux/runtime.c > "$BUILD/runtime1.o"
"$BUILD/c1-stage0" < linux/compiler.c > "$BUILD/compiler1.o"
"$BUILD/c1link" "$BUILD/c1-stage1" _start \
    "$BUILD/startup_sysv.o" "$BUILD/runtime1.o" \
    "$BUILD/compiler1.o" "$BUILD/syscalls_sysv.o"

"$BUILD/c1-stage1" < linux/runtime.c > "$BUILD/runtime2.o"
"$BUILD/c1-stage1" < linux/compiler.c > "$BUILD/compiler2.o"
cmp "$BUILD/runtime1.o" "$BUILD/runtime2.o"
cmp "$BUILD/compiler1.o" "$BUILD/compiler2.o"

"$BUILD/c1link" "$BUILD/c1-stage2" _start \
    "$BUILD/startup_sysv.o" "$BUILD/runtime2.o" \
    "$BUILD/compiler2.o" "$BUILD/syscalls_sysv.o"
cmp "$BUILD/c1-stage1" "$BUILD/c1-stage2"

"$BUILD/c1-stage2" < examples/hello.c > "$BUILD/hello.o"
"$BUILD/c1link" "$BUILD/hello" _start \
    "$BUILD/startup_sysv.o" "$BUILD/runtime2.o" \
    "$BUILD/hello.o" "$BUILD/syscalls_sysv.o"
"$BUILD/hello"

./tests/run.sh
./tests/run-hosted.sh
./tests/run-interop.sh
./tests/reject/run.sh
./tests/run-env.sh
./tests/run-linker.sh
./tests/run-selflink.sh
./tests/run-linker-selfhost.sh
python3 tools/check_sources.py

echo "PASS: stage1 = stage2; hello; SysV, hosted, interop, reject, env, linker, selflink, linker-selfhost and source checks"
