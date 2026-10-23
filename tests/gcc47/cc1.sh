#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
BUILD=${GCC47_BUILD:-/opt/gcc47-build/gcc}
DEPS=${GCC47_DEPS:-/opt/gcc47-deps/lib}
CC=${CC:-"$ROOT/build/tcc/bit-tcc"}
OUT=${OUT:-"$ROOT/build/gcc47-cc1"}
if [ ! -f "$BUILD/gcc/gtype-desc.h" ]; then
    echo "SKIP: GCC 4.7 build tree not configured: $BUILD" >&2
    exit 0
fi
if [ ! -x "$CC" ]; then
    echo "SKIP: bit-tcc not built: $CC" >&2
    exit 0
fi
rm -rf "$OUT"
mkdir -p "$OUT"
make -C "$BUILD/gcc" -j1 cc1 CC="$CC" >/tmp/bit-gcc47-cc1-build.log 2>&1 || {
    echo "FAIL: cc1 link; see /tmp/bit-gcc47-cc1-build.log" >&2
    exit 1
}
cat > "$OUT/hello.c" <<'EOC'
int main(void) { return 42; }
EOC
LD_LIBRARY_PATH="$DEPS" "$BUILD/gcc/cc1" -quiet "$OUT/hello.c" -o "$OUT/hello.s"
as "$OUT/hello.s" -o "$OUT/hello.o"
gcc "$OUT/hello.o" -o "$OUT/hello"
set +e
"$OUT/hello"
status=$?
set -e
[ "$status" -eq 42 ]
echo "PASS: TCC-linked GCC 4.7.4 cc1 compiles and runs hello"
