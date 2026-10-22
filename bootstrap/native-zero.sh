#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
BUILD=${BUILD:-"$ROOT/build/native-zero"}
(cd "$ROOT" && sha256sum -c bootstrap/linux/c0/native/seeds.sha256)
mkdir -p "$BUILD"
BUILD="$BUILD" "$ROOT/bootstrap/linux/c0/native-c1.sh"
