#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
BUILD=${GCC48_BUILD:-/opt/gcc48-build/gcc}
DEPS=${GCC48_DEPS:-/opt/gcc47-deps/lib}
SRC=${GCC48_SRC:-/opt/gcc-4.8.5}
OUT=${OUT:-"$ROOT/build/gcc48-libstdcxx"}
if [ ! -x "$BUILD/gcc/xgcc" ]; then echo "SKIP: GCC 4.8 xgcc not built" >&2; exit 0; fi
LIB="$BUILD/x86_64-unknown-linux-gnu/libstdc++-v3/src/.libs"
if [ ! -f "$LIB/libstdc++.so" ]; then echo "SKIP: GCC 4.8 libstdc++ not built" >&2; exit 0; fi
mkdir -p "$OUT"
cat > "$OUT/test.cc" <<'EOC'
#include <iostream>
#include <string>
int main() { std::string s = "libstdc++"; std::cout << s << " " << 42 << "\n"; return 0; }
EOC
LD_LIBRARY_PATH="$DEPS" "$BUILD/gcc/xgcc" -B"$BUILD/gcc" -B/usr/lib/gcc/x86_64-linux-gnu/13 \
  -I"$SRC/libstdc++-v3/libsupc++" -I"$SRC/libstdc++-v3/include/std" \
  -I"$SRC/libstdc++-v3/include" \
  -I"$BUILD/x86_64-unknown-linux-gnu/libstdc++-v3/include/x86_64-unknown-linux-gnu" \
  -I"$BUILD/x86_64-unknown-linux-gnu/libstdc++-v3/include" \
  -L"$LIB" -o "$OUT/test" "$OUT/test.cc" -lstdc++
LD_LIBRARY_PATH="$LIB:$DEPS" "$OUT/test" | grep -qx 'libstdc++ 42'
echo "PASS: GCC 4.8.5 C++ program runs with built libstdc++"
