#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
BUILD=${BUILD:-"$ROOT/build/fuzz-float"}
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
    name="float-$seed"
    awk -v seed="$seed" 'BEGIN {
      srand(seed);
      print "long double f(long double a, long double b) {";
      print "  long double x = a, y = b;";
      for (i = 0; i < 16; i++) {
        k = int(rand() * 7) + 1;
        op = int(rand() * 4);
        if (op == 0) printf "  x = (x + %d.5L) * y;\n", k;
        else if (op == 1) printf "  y = (y - %d.25L) / (x + 1.0L);\n", k;
        else if (op == 2) printf "  x = x * %d.125L + y;\n", k;
        else printf "  y = (x - %d.75L) * 0.5L;\n", k;
      }
      print "  return x + y;";
      print "}";
      print "int main(void) {";
      printf "  long double v = f(%d.5L, %d.25L);\n", seed + 1, seed + 2;
      print "  return (int)v & 255;";
      print "}";
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
echo "fuzz-float: $pass passed, $fail failed"
test "$fail" -eq 0
