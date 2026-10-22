#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
BUILD=${BUILD:-"$ROOT/build"}
C1="$BUILD/native-c1/c1.elf"
[ -x "$C1" ] || { echo "run-native-hosted: missing $C1" >&2; exit 1; }
C1="$C1" BUILD="$BUILD" "$ROOT/tests/run-hosted.sh"
C1="$C1" BUILD="$BUILD" "$ROOT/tests/run-interop.sh"
C1="$C1" BUILD="$BUILD" "$ROOT/tests/reject/run.sh"
C1="$C1" BUILD="$BUILD" "$ROOT/tests/run-env.sh"
echo "PASS: Native C1 passed hosted and interop suites"
