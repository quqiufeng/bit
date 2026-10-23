#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
BUILD=${GCC47_BUILD:-/opt/gcc47-build/gcc}
DEPS=${GCC47_DEPS:-/opt/gcc47-deps/lib}
S2=${GCC47_STAGE2_CC1:-/tmp/gcc47-stage2-cc1}
OUT=${OUT:-"$ROOT/build/gcc47-stage3"}
if [ ! -x "$BUILD/gcc/cc1" ] || [ ! -x "$S2" ]; then
    echo "SKIP: stage2/stage3 cc1 not present" >&2; exit 0
fi
mkdir -p "$OUT"
printf 'int main(void){return 42;}\n' > "$OUT/hello.c"
LD_LIBRARY_PATH="$DEPS" "$BUILD/gcc/cc1" -quiet "$OUT/hello.c" -o "$OUT/stage3.s"
LD_LIBRARY_PATH="$DEPS" "$S2" -quiet "$OUT/hello.c" -o "$OUT/stage2.s"
cmp "$OUT/stage2.s" "$OUT/stage3.s"
as "$OUT/stage3.s" -o "$OUT/hello.o"
gcc "$OUT/hello.o" -o "$OUT/hello"
set +e; "$OUT/hello"; s=$?; set -e
[ "$s" -eq 42 ]
echo "PASS: GCC 4.7.4 stage2 and stage3 cc1 produce identical assembly"
