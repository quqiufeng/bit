#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
BUILD=${BUILD:-"$ROOT/build/fuzz"}
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
    name="fuzz-$seed"
    awk -v seed="$seed" 'BEGIN {
      srand(seed);
      print "int f(int a, int b) {";
      print "  int x = a, y = b, z = 0;";
      for (i = 0; i < 32; i++) {
        op = int(rand() * 8);
        k = int(rand() * 17) + 1;
        if (op == 0) printf "  x = (x + %d) ^ y;\n", k;
        else if (op == 1) printf "  y = (y * %d) + x;\n", k;
        else if (op == 2) printf "  z = (x - y) * %d;\n", k;
        else if (op == 3) printf "  x = (x & %d) | (y ^ %d);\n", k, k + 1;
        else if (op == 4) printf "  y = (y >> 1) + (x << 1);\n";
        else if (op == 5) printf "  z = z + (x ^ y) + %d;\n", k;
        else if (op == 6) printf "  x = x + y * %d - z;\n", k;
        else printf "  y = y + (x ^ %d) - z;\n", k;
      }
      print "  return (x ^ y ^ z) & 255;";
      print "}";
      print "int main(void) { return f(7, 11); }";
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
echo "fuzz-expr: $pass passed, $fail failed"
test "$fail" -eq 0
