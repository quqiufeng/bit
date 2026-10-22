#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
BUILD=${BUILD:-"$ROOT/build/native-zero"}
mkdir -p "$BUILD"
BUILD="$BUILD" "$ROOT/bootstrap/linux/c0/native-c1.sh"
