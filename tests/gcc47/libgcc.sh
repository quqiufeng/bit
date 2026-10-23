#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
BUILD=${GCC47_BUILD:-/opt/gcc47-build/gcc}
DEPS=${GCC47_DEPS:-/opt/gcc47-deps/lib}
CC=${CC:-/tmp/xgcc-stage2}
OUT=${OUT:-"$ROOT/build/gcc47-libgcc"}
if [ ! -x "$CC" ]; then echo "SKIP: xgcc stage2 wrapper not found: $CC" >&2; exit 0; fi
cd "$BUILD"
LD_LIBRARY_PATH="$DEPS" make -j2 all-target-libgcc CC="$CC" CC_FOR_BUILD=gcc LDFLAGS=-Wl,-no-pie >/tmp/bit-gcc47-libgcc.log 2>&1 || {
    echo "FAIL: libgcc build; see /tmp/bit-gcc47-libgcc.log" >&2; exit 1; }
cat > "$OUT.c" <<'EOC'
int main(void) { return 42; }
EOC
LD_LIBRARY_PATH="$DEPS" "$BUILD/gcc/xgcc" -B"$BUILD/gcc" -B/usr/lib/gcc/x86_64-linux-gnu/13 -o "$OUT" "$OUT.c"
set +e; "$OUT"; status=$?; set -e
[ "$status" -eq 42 ]
echo "PASS: GCC 4.7.4 libgcc built and linked by stage1 GCC"
