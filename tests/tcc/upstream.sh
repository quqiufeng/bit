#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
TCC_SRC=${TCC_SRC:-/opt/tcc-0.9.27}
C1=${C1:-"$ROOT/build/native-c1/c1.elf"}
OUT=${OUT:-"$ROOT/build/tcc"}
INC="$ROOT/include"
if [ ! -f "$TCC_SRC/tcc.c" ]; then
    echo "SKIP: TCC source not found: $TCC_SRC" >&2
    exit 0
fi
if [ ! -x "$C1" ]; then
    echo "SKIP: native C1 not built" >&2
    exit 0
fi
mkdir -p "$OUT"
gcc -c "$ROOT/tools/c1-init.c" -o "$OUT/c1-init.o"
C1_INCLUDE="$TCC_SRC;$INC" "$C1" < "$TCC_SRC/tcc.c" > "$OUT/tcc.o"
gcc -o "$OUT/bit-tcc" "$OUT/tcc.o" "$OUT/c1-init.o" -ldl -lm
"$OUT/bit-tcc" -v >/dev/null
"$OUT/bit-tcc" -c "$ROOT/examples/hello.c" -o "$OUT/hello.o"
gcc -Wl,-z,noexecstack "$OUT/hello.o" -o "$OUT/hello"
"$OUT/hello" >/dev/null
"$OUT/bit-tcc" -nostdinc -c "$TCC_SRC/tcc.c" -o "$OUT/tcc-self1.o" \
    -I"$TCC_SRC" -I"$TCC_SRC/include" -I"$INC"
"$OUT/bit-tcc" -nostdinc -c "$TCC_SRC/tcc.c" -o "$OUT/tcc-self2.o" \
    -I"$TCC_SRC" -I"$TCC_SRC/include" -I"$INC"
cmp "$OUT/tcc-self1.o" "$OUT/tcc-self2.o"
gcc -O2 -c "$TCC_SRC/lib/libtcc1.c" -o "$OUT/libtcc1.o"
gcc -O2 -c "$TCC_SRC/lib/va_list.c" -o "$OUT/va_list.o"
gcc -Wl,-z,noexecstack -o "$OUT/tcc-stage2" "$OUT/tcc-self1.o" "$OUT/libtcc1.o" "$OUT/va_list.o" -ldl -lm
"$OUT/tcc-stage2" -v >/dev/null
echo "PASS: C1 -> TCC -> TCC self-host"
