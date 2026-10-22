#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
BUILD=${BUILD:-"$ROOT/build"}
cmp "$BUILD/native-c1/compiler.o" "$BUILD/compiler1.o"
cmp "$BUILD/native-c1/runtime.o" "$BUILD/runtime1.o"
cmp "$BUILD/native-c1/compiler-stage2.o" "$BUILD/compiler2.o"
echo "PASS: DDC Native C1 vs gcc-built C1 produced identical objects/compiler"
