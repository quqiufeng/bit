#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
BUILD=${GCC47_BUILD:-/opt/gcc47-build/gcc}
SRC=${GCC47_SRC:-/opt/gcc-4.7.4}
DEPS=${GCC47_DEPS:-/opt/gcc47-deps/lib}
C1=${GCC47_STAGE1_CC1:-/tmp/gcc47-stage1-cc1}
X1=${GCC47_STAGE1_XGCC:-/tmp/gcc47-stage1-xgcc}
OUT=${OUT:-"$ROOT/build/gcc47-stage2"}
if [ ! -x "$C1" ] || [ ! -x "$X1" ]; then
    echo "SKIP: stage1 cc1/xgcc snapshots not found" >&2
    exit 0
fi
if [ ! -f "$BUILD/gcc/gtype-desc.h" ]; then
    echo "SKIP: GCC 4.7 build tree not configured" >&2
    exit 0
fi
cp -f "$C1" "$BUILD/gcc/cc1"
cp -f "$X1" "$BUILD/gcc/xgcc"
cat > "$OUT-xgcc" <<EOX
#!/bin/sh
LD_LIBRARY_PATH=$DEPS exec $X1 -B$BUILD/gcc -B/usr/lib/gcc/x86_64-linux-gnu/13 "\$@"
EOX
chmod +x "$OUT-xgcc"
find "$BUILD/gcc" -name '*.o' -delete
rm -f "$BUILD/gcc/libbackend.a" "$BUILD/gcc/libcommon.a" "$BUILD/gcc/libcommon-target.a"
make -C "$BUILD/gcc" -j2 cc1 CC="$OUT-xgcc" CC_FOR_BUILD=gcc LDFLAGS=-Wl,-no-pie >/tmp/bit-gcc47-stage2-build.log 2>&1 || {
    echo "FAIL: stage2 build; see /tmp/bit-gcc47-stage2-build.log" >&2
    exit 1
}
mkdir -p "$OUT"
cat > "$OUT/hello.c" <<'EOC'
int main(void) { return 42; }
EOC
LD_LIBRARY_PATH="$DEPS" "$BUILD/gcc/cc1" -quiet "$OUT/hello.c" -o "$OUT/hello.s"
as "$OUT/hello.s" -o "$OUT/hello.o"
gcc "$OUT/hello.o" -o "$OUT/hello"
set +e
"$OUT/hello"; status=$?
set -e
[ "$status" -eq 42 ]
echo "PASS: GCC 4.7.4 stage2 cc1 built by TCC-built stage1 GCC"
