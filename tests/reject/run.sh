#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname "$0")/../.." && pwd)
BUILD=${BUILD:-"$ROOT/build"}
for source in "$ROOT"/tests/reject/*.c; do
    name=$(basename "$source" .c)
    if "$BUILD/c1-stage2" < "$source" > "$BUILD/reject-$name.o" 2> "$BUILD/reject-$name.err"; then
        echo "FAIL: expected rejection for $name" >&2
        exit 1
    fi
    echo "PASS: reject/$name"
done
