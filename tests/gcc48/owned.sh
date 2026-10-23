#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
BUILD=${GCC48_OWNED_BUILD:-/opt/gcc48-owned-build/gcc}
DEPS=${GCC48_DEPS:-/opt/gcc47-deps/lib}
OUT=${OUT:-"$ROOT/build/gcc48-owned"}
if [ ! -x "$BUILD/gcc/cc1" ] || [ ! -x "$BUILD/gcc/cc1plus" ]; then
    echo "SKIP: owned GCC 4.8 build not found" >&2; exit 0
fi
if ! grep -q '^CC = /tmp/gcc47-cc' "$BUILD/Makefile" || \
   ! grep -q '^CXX = /tmp/gcc47-cxx' "$BUILD/Makefile"; then
    echo "FAIL: GCC 4.8 was not configured with GCC 4.7 wrappers" >&2; exit 1
fi
mkdir -p "$OUT"
printf 'int main(void){return 42;}\n' > "$OUT/c.c"
printf 'int main(){return 42;}\n' > "$OUT/cpp.cc"
LD_LIBRARY_PATH="$DEPS" "$BUILD/gcc/cc1" -quiet "$OUT/c.c" -o "$OUT/c.s"
as "$OUT/c.s" -o "$OUT/c.o"; gcc "$OUT/c.o" -o "$OUT/c"
LD_LIBRARY_PATH="$DEPS" "$BUILD/gcc/cc1plus" -quiet "$OUT/cpp.cc" -o "$OUT/cpp.s"
as "$OUT/cpp.s" -o "$OUT/cpp.o"; gcc "$OUT/cpp.o" -o "$OUT/cpp"
set +e; "$OUT/c"; a=$?; "$OUT/cpp"; b=$?; set -e
[ "$a" -eq 42 ] && [ "$b" -eq 42 ]
echo "PASS: GCC 4.8.5 built by GCC 4.7 C++ bridge, C and C++ hello run"
