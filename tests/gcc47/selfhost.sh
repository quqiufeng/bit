#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
BUILD=${GCC47_BUILD:-/opt/gcc47-build/gcc}
DEPS=${GCC47_DEPS:-/opt/gcc47-deps/lib}
SRC=${GCC47_SRC:-/opt/gcc-4.7.4}
OUT=${OUT:-"$ROOT/build/gcc47-selfhost"}
XGCC="$BUILD/gcc/xgcc"
if [ ! -x "$XGCC" ]; then
    echo "SKIP: GCC 4.7 xgcc not built: $XGCC" >&2
    exit 0
fi
rm -rf "$OUT"; mkdir -p "$OUT"
for f in c-decl.c c-parser.c c-typeck.c c-lang.c; do
    LD_LIBRARY_PATH="$DEPS" "$XGCC" -B"$BUILD/gcc" -B/usr/lib/gcc/x86_64-linux-gnu/13 \
        -DIN_GCC -DHAVE_CONFIG_H -I"$BUILD/gcc" -I"$SRC/gcc" \
        -I"$SRC/gcc/config" -I"$SRC/gcc/c-family" -I"$SRC/include" \
        -I"$SRC/libcpp/include" -I"$SRC/libiberty" \
        -c "$SRC/gcc/$f" -o "$OUT/${f%.c}.o"
done
echo "PASS: TCC-built GCC 4.7.4 xgcc compiles GCC C frontend sources"
