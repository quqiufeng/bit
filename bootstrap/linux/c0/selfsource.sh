#!/bin/sh
set -u
dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
ROOT=$(CDPATH= cd -- "$dir/../../.." && pwd)
BUILD=${BUILD:-"$ROOT/build"}
out=${TMPDIR:-/tmp}/c0-selfsource.$$
"$BUILD/c0/c0" < "$dir/c0.c" > "$out" 2>"$out.err"
status=$?
rm -f "$out"
if [ "$status" -eq 0 ]; then
    echo "PASS: C0 compiles its own C source"
    exit 0
fi
cat "$out.err"
exit 1
