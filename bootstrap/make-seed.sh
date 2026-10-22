#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
BUILD=${BUILD:-"$ROOT/build"}
OUT=${1:-"$ROOT/bootstrap"}
for spec in \
    seed-c1.hex:c1-stage2 \
    seed-link.hex:c1link \
    seed-startup.hex:startup_sysv.o \
    seed-syscalls.hex:syscalls_sysv.o; do
    name=${spec%%:*}
    artifact=${spec#*:}
    xxd -p -c 32 "$BUILD/$artifact" > "$OUT/$name"
done
