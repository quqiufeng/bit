#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
BUILD=${GCC47_BUILD:-/opt/gcc47-build/gcc}
DEPS=${GCC47_DEPS:-/opt/gcc47-deps/lib}
CC=${CC:-"$ROOT/build/tcc/bit-tcc"}
SYSGCC=${SYSGCC:-/usr/lib/gcc/x86_64-linux-gnu/13}
OUT=${OUT:-"$ROOT/build/gcc47-xgcc"}
if [ ! -f "$BUILD/gcc/gtype-desc.h" ]; then
    echo "SKIP: GCC 4.7 build tree not configured: $BUILD" >&2
    exit 0
fi
mkdir -p "$OUT"
printf 'void *__dso_handle;\n' > "$OUT/dso.c"
"$CC" -c "$OUT/dso.c" -o "$OUT/dso.o"
make -C "$BUILD/gcc" -j1 xgcc CC="$CC" LDFLAGS="$OUT/dso.o" >/tmp/bit-gcc47-xgcc-build.log 2>&1 || {
    echo "FAIL: xgcc link; see /tmp/bit-gcc47-xgcc-build.log" >&2
    exit 1
}
cat > "$OUT/hello.c" <<'EOC'
int main(void) { return 42; }
EOC
LD_LIBRARY_PATH="$DEPS" "$BUILD/gcc/xgcc" -B"$BUILD/gcc" -B"$SYSGCC" -L"$SYSGCC" \
    -o "$OUT/hello" "$OUT/hello.c"
set +e
"$OUT/hello"
status=$?
set -e
[ "$status" -eq 42 ]
echo "PASS: TCC-linked GCC 4.7.4 xgcc compiles and runs hello"
