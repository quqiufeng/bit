#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
BUILD=${BUILD:-"$ROOT/build/fuzz-abi-float"}
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
for n in 1 2 3 4 5 6 7 8 9 10; do
    name="abi-float-$n"
    awk -v n="$n" 'BEGIN {
      printf "long f(";
      for (i = 0; i < n; i++) {
        if (i) printf ", ";
        if (i % 3 == 0) printf "int a%d", i;
        else if (i % 3 == 1) printf "double a%d", i;
        else printf "float a%d", i;
      }
      printf ") {\n  long s = 0;\n";
      for (i = 0; i < n; i++) {
        if (i % 3 == 0) printf "  s = s * 3 + a%d;\n", i;
        else printf "  s = s * 3 + (long)(a%d * 2);\n", i;
      }
      print "  return s;";
      print "}";
      printf "int main(void) { return (int)f(";
      for (i = 0; i < n; i++) {
        if (i) printf ", ";
        if (i % 3 == 0) printf "%d", (i * 11 + 3) % 41 - 20;
        else printf "%d.5", (i * 7 + 1) % 17;
      }
      print ") & 255;";
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
echo "fuzz-abi-float: $pass passed, $fail failed"
test "$fail" -eq 0
