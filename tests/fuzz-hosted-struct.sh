#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
BUILD=${BUILD:-"$ROOT/build/fuzz-hosted-struct"}
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
    name="hosted-struct-$seed"
    awk -v seed="$seed" 'BEGIN {
      print "#include <stdlib.h>";
      print "int main(void) {";
      printf "  div_t d = div(%d, %d);\n", seed * 17 + 3, seed + 3;
      printf "  ldiv_t l = ldiv(%d, %d);\n", seed * 1000 + 7, seed + 7;
      print "  if (d.quot * d.rem != d.quot * (d.quot ? d.rem : 0)) return 1;";
      print "  if (l.quot < 0 || l.rem < 0) return 2;";
      print "  return (int)(d.quot + d.rem + l.quot + l.rem) & 255;";
      print "}";
    }' > "$BUILD/cases/$name.c"
    C1_INCLUDE="$ROOT/include" "$C1" < "$BUILD/cases/$name.c" > "$BUILD/cases/$name.o"
    "$ROOT/linux/link-hosted.sh" "$BUILD/cases/$name.native" "$BUILD/cases/$name.o"
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
echo "fuzz-hosted-struct: $pass passed, $fail failed"
test "$fail" -eq 0
