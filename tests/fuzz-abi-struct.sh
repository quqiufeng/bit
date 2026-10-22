#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
BUILD=${BUILD:-"$ROOT/build/fuzz-abi-struct"}
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
    name="abi-struct-$seed"
    awk -v seed="$seed" 'BEGIN {
      srand(seed);
      n = (seed % 2) ? 4 : 6;
      types[0] = "char"; types[1] = "short"; types[2] = "int"; types[3] = "long"; types[4] = "int"; types[5] = "long";
      names[0] = "a"; names[1] = "b"; names[2] = "c"; names[3] = "d"; names[4] = "e"; names[5] = "f";
      for (i = 0; i < n; i++) order[i] = i;
      for (i = n - 1; i > 0; i--) { j = int(rand() * (i + 1)); t = order[i]; order[i] = order[j]; order[j] = t; }
      print "struct S {";
      for (i = 0; i < n; i++) printf "  %s %s;\n", types[order[i]], names[order[i]];
      print "};";
      print "struct S make(void) { struct S s;";
      for (i = 0; i < n; i++) printf "  s.%s = %d;\n", names[i], (i * 7 + seed * 11) % 101 - 50;
      print "  return s;";
      print "}";
      print "long use(struct S s) {";
      print "  long r = 0;";
      for (i = 0; i < n; i++) printf "  r = r * 3 + s.%s;\n", names[i];
      print "  return r;";
      print "}";
      print "int main(void) { struct S s = make(); return (int)use(s) & 255; }";
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
echo "fuzz-abi-struct: $pass passed, $fail failed"
test "$fail" -eq 0
