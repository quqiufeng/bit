#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
BUILD=${GCC142_OWNED_BUILD:-/opt/gcc142-owned-build/gcc}
DEPS=${GCC142_DEPS:-/opt/gcc47-deps/lib}
SRC=${GCC142_SRC:-/opt/gcc-14.2.0}
OUT=${OUT:-"$ROOT/build/gcc142-owned-libstdcxx"}
LIB="$BUILD/x86_64-pc-linux-gnu/libstdc++-v3/src/.libs"
if [ ! -f "$LIB/libstdc++.so" ]; then echo "SKIP: GCC 14 owned libstdc++" >&2; exit 0; fi
mkdir -p "$OUT"; printf '#include <iostream>\nint main(){std::cout<<"gcc14-owned"<<"\\n";return 0;}\n' > "$OUT/t.cc"
LD_LIBRARY_PATH="$DEPS" "$BUILD/gcc/xgcc" -B"$BUILD/gcc" -B/usr/lib/gcc/x86_64-linux-gnu/13 \
 -I"$SRC/libstdc++-v3/libsupc++" -I"$SRC/libstdc++-v3/include/std" -I"$SRC/libstdc++-v3/include" \
 -I"$BUILD/x86_64-pc-linux-gnu/libstdc++-v3/include/x86_64-pc-linux-gnu" \
 -I"$BUILD/x86_64-pc-linux-gnu/libstdc++-v3/include" -L"$LIB" -o "$OUT/t" "$OUT/t.cc" -lstdc++
LD_LIBRARY_PATH="$LIB:$DEPS" "$OUT/t" | grep -qx 'gcc14-owned'
echo "PASS: GCC 14.2.0 built by 13.3 runs C++ iostream"
