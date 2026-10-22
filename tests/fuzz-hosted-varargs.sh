#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
BUILD=${BUILD:-"$ROOT/build/fuzz-hosted-va"}
OUT="$BUILD/native-c1"
if [ ! -x "$OUT/c1.elf" ]; then
    BUILD="$BUILD" "$ROOT/build-zero.sh" >/dev/null
fi
C1="$OUT/c1.elf"
CC=${CC:-gcc}
mkdir -p "$BUILD/cases"
pass=0
fail=0
for seed in 1 2 3 4 5 6 7 8; do
    name="hosted-va-$seed"
    awk -v seed="$seed" 'BEGIN {
      print "#include <stdio.h>";
      print "int main(void) {";
      printf "  printf(\"%%d %%.2f %%Lf %%d %%.2f %%Lf\\n\", %d, %d.5, %d.25L, %d, %d.5, %d.25L);\n", seed, seed, seed, seed + 1, seed + 1, seed + 1;
      print "  return 0;";
      print "}";
    }' > "$BUILD/cases/$name.c"
    C1_INCLUDE="$ROOT/include" "$C1" < "$BUILD/cases/$name.c" > "$BUILD/cases/$name.o"
    "$ROOT/linux/link-hosted.sh" "$BUILD/cases/$name.native" "$BUILD/cases/$name.o"
    "$CC" -O0 -fno-pic -fno-pie -no-pie "$BUILD/cases/$name.c" -o "$BUILD/cases/$name.gcc"
    set +e
    "$BUILD/cases/$name.native" > "$BUILD/cases/$name.native.out" 2>/dev/null; ns=$?
    "$BUILD/cases/$name.gcc" > "$BUILD/cases/$name.gcc.out" 2>/dev/null; gs=$?
    set -e
    if [ "$ns" -ne "$gs" ] || ! cmp -s "$BUILD/cases/$name.native.out" "$BUILD/cases/$name.gcc.out"; then
        echo "FAIL: $name native=$ns gcc=$gs"
        fail=$((fail + 1))
    else
        pass=$((pass + 1))
    fi
done
echo "fuzz-hosted-varargs: $pass passed, $fail failed"
test "$fail" -eq 0
