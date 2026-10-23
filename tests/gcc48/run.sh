#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
BUILD=${GCC48_BUILD:-/opt/gcc48-build/gcc}
DEPS=${GCC48_DEPS:-/opt/gcc47-deps/lib}
OUT=${OUT:-"$ROOT/build/gcc48"}
if [ ! -x "$BUILD/gcc/cc1" ] || [ ! -x "$BUILD/gcc/cc1plus" ]; then
    echo "SKIP: GCC 4.8 build tree not found: $BUILD" >&2
    exit 0
fi
mkdir -p "$OUT"
printf 'int main(void){return 42;}\n' > "$OUT/hello.c"
printf 'int main(){return 42;}\n' > "$OUT/hello.cc"
LD_LIBRARY_PATH="$DEPS" "$BUILD/gcc/cc1" -quiet "$OUT/hello.c" -o "$OUT/hello-c.s"
as "$OUT/hello-c.s" -o "$OUT/hello-c.o"; gcc "$OUT/hello-c.o" -o "$OUT/hello-c"
LD_LIBRARY_PATH="$DEPS" "$BUILD/gcc/cc1plus" -quiet "$OUT/hello.cc" -o "$OUT/hello-cpp.s"
as "$OUT/hello-cpp.s" -o "$OUT/hello-cpp.o"; gcc "$OUT/hello-cpp.o" -o "$OUT/hello-cpp"
set +e; "$OUT/hello-c"; a=$?; "$OUT/hello-cpp"; b=$?; set -e
[ "$a" -eq 42 ] && [ "$b" -eq 42 ]
echo "PASS: GCC 4.8.5 cc1 and cc1plus compile C and C++ hello"
