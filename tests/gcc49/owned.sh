#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
BUILD=${GCC49_OWNED_BUILD:-/opt/gcc49-owned-build/gcc}
DEPS=${GCC49_DEPS:-/opt/gcc47-deps/lib}
OUT=${OUT:-"$ROOT/build/gcc49-owned"}
if [ ! -x "$BUILD/gcc/cc1" ] || [ ! -x "$BUILD/gcc/cc1plus" ]; then echo "SKIP: owned GCC 4.9 build"; exit 0; fi
if ! grep -q '^CC = /tmp/gcc48-cc' "$BUILD/Makefile" || ! grep -q '^CXX = /tmp/gcc48-cxx' "$BUILD/Makefile"; then echo "FAIL: GCC 4.9 not built by GCC 4.8 wrappers" >&2; exit 1; fi
mkdir -p "$OUT"
printf 'int main(void){return 42;}\n' > "$OUT/c.c"
printf 'int main(){return 42;}\n' > "$OUT/cpp.cc"
LD_LIBRARY_PATH="$DEPS" "$BUILD/gcc/cc1" -quiet "$OUT/c.c" -o "$OUT/c.s"
as "$OUT/c.s" -o "$OUT/c.o"; gcc "$OUT/c.o" -o "$OUT/c"
LD_LIBRARY_PATH="$DEPS" "$BUILD/gcc/cc1plus" -quiet "$OUT/cpp.cc" -o "$OUT/cpp.s"
as "$OUT/cpp.s" -o "$OUT/cpp.o"; gcc "$OUT/cpp.o" -o "$OUT/cpp"
set +e; "$OUT/c"; a=$?; "$OUT/cpp"; b=$?; set -e
[ "$a" -eq 42 ] && [ "$b" -eq 42 ]
echo "PASS: GCC 4.9.4 built by GCC 4.8 C++ bridge, C and C++ hello run"
