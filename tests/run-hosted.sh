#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)
BUILD=${BUILD:-"$ROOT/build"}
for source in "$ROOT"/tests/hosted/*.c; do
    name=$(basename "$source" .c)
    "$BUILD/c1-stage2" < "$source" > "$BUILD/hosted-$name.o"
    "$ROOT/linux/link-hosted.sh" "$BUILD/hosted-$name" "$BUILD/hosted-$name.o"
    "$BUILD/hosted-$name"
    echo "PASS: hosted/$name"
done
