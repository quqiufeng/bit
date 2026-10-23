#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
BUILD=${GCC55_BUILD:-/opt/gcc55-build/gcc}
DEPS=${GCC55_DEPS:-/opt/gcc47-deps/lib}
OUT=${OUT:-"$ROOT/build/gcc55"}
if [ ! -x "$BUILD/gcc/cc1" ] || [ ! -x "$BUILD/gcc/cc1plus" ]; then echo "SKIP: GCC 5.5 build tree"; exit 0; fi
mkdir -p "$OUT"
printf 'int main(void){return 42;}\n' > "$OUT/h.c"
printf 'int main(){return 42;}\n' > "$OUT/h.cc"
LD_LIBRARY_PATH="$DEPS" "$BUILD/gcc/cc1" -quiet "$OUT/h.c" -o "$OUT/c.s"
as "$OUT/c.s" -o "$OUT/c.o"; gcc "$OUT/c.o" -o "$OUT/c"
LD_LIBRARY_PATH="$DEPS" "$BUILD/gcc/cc1plus" -quiet "$OUT/h.cc" -o "$OUT/cpp.s"
as "$OUT/cpp.s" -o "$OUT/cpp.o"; gcc "$OUT/cpp.o" -o "$OUT/cpp"
set +e; "$OUT/c"; a=$?; "$OUT/cpp"; b=$?; set -e
[ "$a" -eq 42 ] && [ "$b" -eq 42 ]
echo "PASS: GCC 5.5.0 cc1 and cc1plus compile C and C++ hello"
