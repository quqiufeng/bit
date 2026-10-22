#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname "$0")/../.." && pwd)
BUILD=${BUILD:-"$ROOT/build"}
C1=${C1:-$C1}
for source in "$ROOT"/tests/reject/*.c; do
    name=$(basename "$source" .c)
    if $C1 < "$source" > "$BUILD/reject-$name.o" 2> "$BUILD/reject-$name.err"; then
        echo "FAIL: expected rejection for $name" >&2
        exit 1
    fi
    echo "PASS: reject/$name"
done
