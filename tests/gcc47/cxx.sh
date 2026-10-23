#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
BUILD=${GCC47_CXX_BUILD:-/opt/gcc47-cxx-build/gcc}
DEPS=${GCC47_DEPS:-/opt/gcc47-deps/lib}
OUT=${OUT:-"$ROOT/build/gcc47-cxx"}
if [ ! -x "$BUILD/gcc/cc1plus" ]; then echo "SKIP: GCC 4.7 cc1plus not built" >&2; exit 0; fi
mkdir -p "$OUT"
printf 'int main(){return 42;}\n' > "$OUT/hello.cc"
LD_LIBRARY_PATH="$DEPS" "$BUILD/gcc/cc1plus" -quiet "$OUT/hello.cc" -o "$OUT/hello.s"
as "$OUT/hello.s" -o "$OUT/hello.o"; gcc "$OUT/hello.o" -o "$OUT/hello"
set +e; "$OUT/hello"; s=$?; set -e
[ "$s" -eq 42 ]
echo "PASS: GCC 4.7.4 cc1plus compiles C++ hello"
