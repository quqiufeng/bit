#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)
BUILD=${BUILD:-"$ROOT/build"}
LD=${LD:-ld}
LDFLAGS="-static -nostdlib -z noexecstack -e _start"

for source in "$ROOT"/tests/sysv/*.c; do
    name=$(basename "$source" .c)
    "$BUILD/c1-stage2" < "$source" > "$BUILD/test-$name.o"
    "$LD" $LDFLAGS -o "$BUILD/test-$name" \
        "$BUILD/startup_sysv.o" "$BUILD/runtime2.o" \
        "$BUILD/test-$name.o" "$BUILD/syscalls_sysv.o"
    "$BUILD/test-$name"
    echo "PASS: sysv/$name"
done
