#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
BUILD=${GCC49_OWNED_BUILD:-/opt/gcc49-owned-build/gcc}
DEPS=${GCC49_DEPS:-/opt/gcc47-deps/lib}
SRC=${GCC49_SRC:-/opt/gcc-4.9.4}
OUT=${OUT:-"$ROOT/build/gcc49-owned-libstdcxx"}
if [ ! -x "$BUILD/gcc/xgcc" ]; then echo "SKIP: GCC 4.9 owned xgcc" >&2; exit 0; fi
LIB="$BUILD/x86_64-unknown-linux-gnu/libstdc++-v3/src/.libs"
if [ ! -f "$LIB/libstdc++.so" ]; then echo "SKIP: GCC 4.9 owned libstdc++" >&2; exit 0; fi
mkdir -p "$OUT"
printf '#include <iostream>\nint main(){std::cout<<"gcc49-owned"<<"\\n";return 0;}\n' > "$OUT/t.cc"
LD_LIBRARY_PATH="$DEPS" "$BUILD/gcc/xgcc" -B"$BUILD/gcc" -B/usr/lib/gcc/x86_64-linux-gnu/13 \
 -I"$SRC/libstdc++-v3/libsupc++" -I"$SRC/libstdc++-v3/include/std" -I"$SRC/libstdc++-v3/include" \
 -I"$BUILD/x86_64-unknown-linux-gnu/libstdc++-v3/include/x86_64-unknown-linux-gnu" \
 -I"$BUILD/x86_64-unknown-linux-gnu/libstdc++-v3/include" \
 -L"$LIB" -o "$OUT/t" "$OUT/t.cc" -lstdc++
LD_LIBRARY_PATH="$LIB:$DEPS" "$OUT/t" | grep -qx 'gcc49-owned'
echo "PASS: GCC 4.9.4 built by 4.8 runs C++ iostream"
