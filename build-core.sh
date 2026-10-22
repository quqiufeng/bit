#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
BUILD=${BUILD:-"$ROOT/build/core"}
BUILD="$BUILD" "$ROOT/build-zero.sh"
BUILD="$BUILD" "$ROOT/tests/run-native.sh"
echo "PASS: zero-dependency core build and native SysV corpus"
