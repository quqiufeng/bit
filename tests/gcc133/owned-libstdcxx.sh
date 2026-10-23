#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
BUILD=${GCC133_OWNED_BUILD:-/opt/gcc133-owned-build/gcc}
DEPS=${GCC133_DEPS:-/opt/gcc47-deps/lib}
SRC=${GCC133_SRC:-/opt/gcc-13.3.0}
OUT=${OUT:-"$ROOT/build/gcc133-owned-libstdcxx"}
LIB="$BUILD/x86_64-pc-linux-gnu/libstdc++-v3/src/.libs"
if [ ! -f "$LIB/libstdc++.so" ]; then echo "SKIP: GCC 13 owned libstdc++" >&2; exit 0; fi
mkdir -p "$OUT"; printf '#include <iostream>\nint main(){std::cout<<"gcc13-owned"<<"\\n";return 0;}\n' > "$OUT/t.cc"
LD_LIBRARY_PATH="$DEPS" "$BUILD/gcc/xgcc" -B"$BUILD/gcc" -B/usr/lib/gcc/x86_64-linux-gnu/13 \
 -I"$SRC/libstdc++-v3/libsupc++" -I"$SRC/libstdc++-v3/include/std" -I"$SRC/libstdc++-v3/include" \
 -I"$BUILD/x86_64-pc-linux-gnu/libstdc++-v3/include/x86_64-pc-linux-gnu" \
 -I"$BUILD/x86_64-pc-linux-gnu/libstdc++-v3/include" -L"$LIB" -o "$OUT/t" "$OUT/t.cc" -lstdc++
LD_LIBRARY_PATH="$LIB:$DEPS" "$OUT/t" | grep -qx 'gcc13-owned'
echo "PASS: GCC 13.3.0 built by 12.3 runs C++ iostream"
