#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)
BUILD=${BUILD:-"$ROOT/build"}
"$BUILD/c1-stage2" < "$ROOT/tests/shlib/answer.c" > "$BUILD/plugin-answer.o"
"$BUILD/c1link" --shared /tmp/c1-hosted-plugin.so "$BUILD/plugin-answer.o"
"$BUILD/c1-stage2" < "$ROOT/tests/shlib/main.c" > "$BUILD/shared-object.o"
"$ROOT/linux/link-hosted.sh" "$BUILD/shared-object" "$BUILD/shared-object.o"
"$BUILD/shared-object"
echo "PASS: C1 object as shared library"
