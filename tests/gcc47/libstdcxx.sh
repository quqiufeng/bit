#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
BUILD=${GCC47_CXX_BUILD:-/opt/gcc47-cxx-build/gcc}
DEPS=${GCC47_DEPS:-/opt/gcc47-deps/lib}
SRC=${GCC47_SRC:-/opt/gcc-4.7.4}
OUT=${OUT:-"$ROOT/build/gcc47-libstdcxx"}
if [ ! -x "$BUILD/gcc/xgcc" ]; then echo "SKIP: GCC 4.7 xgcc" >&2; exit 0; fi
LIB="$BUILD/x86_64-unknown-linux-gnu/libstdc++-v3/src/.libs"
if [ ! -f "$LIB/libstdc++.so" ]; then echo "SKIP: GCC 4.7 libstdc++" >&2; exit 0; fi
mkdir -p "$OUT"
cat > "$OUT/test.cc" <<'EOC'
#include <iostream>
int main(){ std::cout << "gcc47 c++" << "\n"; return 0; }
EOC
LD_LIBRARY_PATH="$DEPS" "$BUILD/gcc/xgcc" -B"$BUILD/gcc" -B/usr/lib/gcc/x86_64-linux-gnu/13 \
 -I"$SRC/libstdc++-v3/include" -I"$SRC/libstdc++-v3/include/std" -I"$SRC/libstdc++-v3/libsupc++" \
 -I"$BUILD/x86_64-unknown-linux-gnu/libstdc++-v3/include/x86_64-unknown-linux-gnu" \
 -I"$BUILD/x86_64-unknown-linux-gnu/libstdc++-v3/include" \
 -L"$LIB" -o "$OUT/test" "$OUT/test.cc" -lstdc++
LD_LIBRARY_PATH="$LIB:$DEPS" "$OUT/test" | grep -qx 'gcc47 c++'
echo "PASS: GCC 4.7.4 C++ iostream runs with built libstdc++"
