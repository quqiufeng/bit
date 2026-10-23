#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
BUILD=${GCC47_BUILD:-/opt/gcc47-build/gcc}
DEPS=${GCC47_DEPS:-/opt/gcc47-deps/lib}
CC=${CC:-/tmp/xgcc-stage2}
OUT=${OUT:-"$ROOT/build/gcc47-driver"}
if [ ! -x "$CC" ]; then echo "SKIP: stage1 xgcc wrapper not found" >&2; exit 0; fi
if [ ! -f "$BUILD/gcc/libgcc.a" ]; then
    echo "SKIP: GCC 4.7 libgcc not built" >&2; exit 0
fi
rm -f "$BUILD/gcc/xgcc"
make -C "$BUILD/gcc" xgcc CC="$CC" LDFLAGS=-Wl,-no-pie >/tmp/bit-gcc47-driver-build.log 2>&1 || {
    echo "FAIL: stage2 xgcc build; see /tmp/bit-gcc47-driver-build.log" >&2; exit 1; }
cat > "$OUT.c" <<'EOC'
int main(void) { return 42; }
EOC
LD_LIBRARY_PATH="$DEPS" "$BUILD/gcc/xgcc" -B"$BUILD/gcc" -B/usr/lib/gcc/x86_64-linux-gnu/13 \
    -o "$OUT" "$OUT.c"
set +e; "$OUT"; status=$?; set -e
[ "$status" -eq 42 ]
echo "PASS: GCC 4.7.4 stage2 xgcc uses its own libgcc"
