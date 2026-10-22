#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
BUILD=${BUILD:-"$ROOT/build/fuzz-hosted-ld"}
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
    name="hosted-ld-$seed"
    awk -v seed="$seed" 'BEGIN {
      print "long double sinl(long double);";
      print "long double cosl(long double);";
      print "long double sqrtl(long double);";
      print "int main(void) {";
      printf "  long double x = %d.5L;\n", seed + 1;
      print "  int i;";
      print "  for (i = 0; i < 6; i++) {";
      print "    x = sinl(x) + cosl(x) + sqrtl(x + 2.0L);";
      print "    x = x / 3.0L;";
      print "  }";
      print "  return (int)(x * 1000.0L) & 255;";
      print "}";
    }' > "$BUILD/cases/$name.c"
    C1_INCLUDE="$ROOT/include" "$C1" < "$BUILD/cases/$name.c" > "$BUILD/cases/$name.o"
    "$ROOT/linux/link-hosted.sh" "$BUILD/cases/$name.native" "$BUILD/cases/$name.o"
    "$CC" -O0 -fno-pic -fno-pie -no-pie "$BUILD/cases/$name.c" -o "$BUILD/cases/$name.gcc" -lm
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
echo "fuzz-hosted-longdouble: $pass passed, $fail failed"
test "$fail" -eq 0
