#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
BUILD=${GCC123_BUILD:-/opt/gcc123-build/gcc}
DEPS=${GCC123_DEPS:-/opt/gcc47-deps/lib}
SRC=${GCC123_SRC:-/opt/gcc-12.3.0}
OUT=${OUT:-"$ROOT/build/gcc123-libstdcxx"}
if [ ! -x "$BUILD/gcc/xgcc" ]; then echo "SKIP: GCC 12 xgcc" >&2; exit 0; fi
LIB="$BUILD/x86_64-pc-linux-gnu/libstdc++-v3/src/.libs"
if [ ! -f "$LIB/libstdc++.so" ]; then echo "SKIP: libstdc++ not built" >&2; exit 0; fi
mkdir -p "$OUT"
cat > "$OUT/test.cc" <<'EOC'
#include <iostream>
#include <string>
int main(){std::string s="gcc12"; std::cout<<s<<" "<<42<<"\n"; return 0;}
EOC
LD_LIBRARY_PATH="$DEPS" "$BUILD/gcc/xgcc" -B"$BUILD/gcc" -B/usr/lib/gcc/x86_64-linux-gnu/13 \
 -I"$SRC/libstdc++-v3/libsupc++" -I"$SRC/libstdc++-v3/include/std" -I"$SRC/libstdc++-v3/include" \
 -I"$BUILD/x86_64-pc-linux-gnu/libstdc++-v3/include/x86_64-pc-linux-gnu" \
 -I"$BUILD/x86_64-pc-linux-gnu/libstdc++-v3/include" \
 -L"$LIB" -o "$OUT/test" "$OUT/test.cc" -lstdc++
LD_LIBRARY_PATH="$LIB:$DEPS" "$OUT/test" | grep -qx 'gcc12 42'
echo "PASS: GCC 12.3.0 C++ program runs with built libstdc++"
