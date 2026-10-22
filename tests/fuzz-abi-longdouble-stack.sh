#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
BUILD=${BUILD:-"$ROOT/build/fuzz-abi-ld"}
OUT="$BUILD/native-c1"
if [ ! -x "$OUT/c1.elf" ]; then
    BUILD="$BUILD" "$ROOT/build-zero.sh" >/dev/null
fi
C1="$OUT/c1.elf"
LINKER="$OUT/native-linker"
RUNTIME="$OUT/runtime.o"
CC=${CC:-gcc}
mkdir -p "$BUILD/cases"
pass=0
fail=0
for seed in 1 2 3 4 5 6 7 8; do
    name="abi-ld-$seed"
    awk -v seed="$seed" 'BEGIN {
      printf "long f(int a0, int a1, int a2, int a3, int a4, int a5, int a6, long double x, long double y) {\n";
      print "  long s = a0 + a1 + a2 + a3 + a4 + a5 + a6;";
      print "  s = s * 3 + (long)(x * 2);";
      print "  s = s * 3 + (long)(y * 4);";
      print "  return s;";
      print "}";
      printf "int main(void) { return (int)f(%d, %d, %d, %d, %d, %d, %d, %d.5L, %d.25L) & 255; }\n", seed, seed+1, seed+2, seed+3, seed+4, seed+5, seed+6, seed+7, seed+8;
    }' > "$BUILD/cases/$name.c"
    C1_INCLUDE="$ROOT/include" "$C1" < "$BUILD/cases/$name.c" > "$BUILD/cases/$name.o"
    "$LINKER" --bootstrap "$BUILD/cases/$name.native" _start "$RUNTIME" "$BUILD/cases/$name.o" >/dev/null
    "$CC" -O0 -fno-pic -fno-pie -no-pie "$BUILD/cases/$name.c" -o "$BUILD/cases/$name.gcc"
    set +e
    "$BUILD/cases/$name.native"; ns=$?
    "$BUILD/cases/$name.gcc"; gs=$?
    set -e
    if [ "$ns" -ne "$gs" ]; then
        echo "FAIL: $name native=$ns gcc=$gs"
        fail=$((fail + 1))
    else
        pass=$((pass + 1))
    fi
done
echo "fuzz-abi-longdouble-stack: $pass passed, $fail failed"
test "$fail" -eq 0
