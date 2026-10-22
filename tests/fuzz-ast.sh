#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
BUILD=${BUILD:-"$ROOT/build/fuzz-ast"}
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
    name="ast-$seed"
    awk -v seed="$seed" 'function gen(d) {
      if (d == 0) return int(rand() * 9) + 1;
      op = int(rand() * 5);
      if (op == 0) return "(" gen(d-1) " + " gen(d-1) ")";
      if (op == 1) return "(" gen(d-1) " - " gen(d-1) ")";
      if (op == 2) return "(" gen(d-1) " * " gen(d-1) ")";
      if (op == 3) return "(" gen(d-1) " & " gen(d-1) ")";
      return "(" gen(d-1) " ^ " gen(d-1) ")";
    }
    BEGIN {
      srand(seed);
      print "int main(void) {";
      print "  int x = " gen(3) ";";
      print "  return x & 255;";
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
echo "fuzz-ast: $pass passed, $fail failed"
test "$fail" -eq 0
