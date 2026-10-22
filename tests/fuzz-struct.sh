#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
BUILD=${BUILD:-"$ROOT/build/fuzz-struct"}
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
    name="struct-$seed"
    awk -v seed="$seed" 'BEGIN {
      srand(seed);
      fields[0] = "char a;"; fields[1] = "short b;"; fields[2] = "int c;"; fields[3] = "long d;";
      for (i = 0; i < 4; i++) order[i] = i;
      for (i = 3; i > 0; i--) { j = int(rand() * (i + 1)); t = order[i]; order[i] = order[j]; order[j] = t; }
      print "struct S {";
      for (i = 0; i < 4; i++) printf "  %s\n", fields[order[i]];
      print "};";
      print "long sum(struct S s) { return s.a + s.b + s.c + s.d; }";
      print "int main(void) {";
      print "  struct S s;";
      printf "  s.a = %d; s.b = %d; s.c = %d; s.d = %d;\n", (seed * 3) % 100 - 50, (seed * 5) % 200 - 100, (seed * 7) % 1000 - 500, (seed * 11) % 10000 - 5000;
      print "  return (int)sum(s) & 255;";
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
echo "fuzz-struct: $pass passed, $fail failed"
test "$fail" -eq 0
