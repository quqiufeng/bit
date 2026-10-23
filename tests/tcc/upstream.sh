#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
TCC_SRC=${TCC_SRC:-"$ROOT/tcc-0.9.27"}
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
gcc -o "$OUT/bit-tcc-bin" "$OUT/tcc.o" "$OUT/c1-init.o" -ldl -lm
cat > "$OUT/bit-tcc" <<'EOF'
#!/bin/sh
DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
if [ "$#" -eq 1 ] && [ "$1" = "-v" ]; then
    exec "$DIR/bit-tcc-bin" -v
fi
exec "$DIR/bit-tcc-bin" -B"$DIR/runtime" "$@"
EOF
chmod +x "$OUT/bit-tcc"
"$OUT/bit-tcc" -v >/dev/null

# Build the TCC runtime bundle with TCC itself: libtcc1 + headers.
RUNTIME="$OUT/runtime"
rm -rf "$RUNTIME"
mkdir -p "$RUNTIME/include"
cp "$TCC_SRC"/include/*.h "$TCC_SRC"/tcclib.h "$RUNTIME/include/"
"$OUT/bit-tcc" -B "$RUNTIME" -c "$TCC_SRC/lib/libtcc1.c" -o "$RUNTIME/libtcc1.o"
"$OUT/bit-tcc" -B "$RUNTIME" -c "$TCC_SRC/lib/va_list.c" -o "$RUNTIME/va_list.o"
"$OUT/bit-tcc" -B "$RUNTIME" -c "$TCC_SRC/lib/alloca86_64.S" -o "$RUNTIME/alloca86_64.o"
ar rcs "$RUNTIME/libtcc1.a" "$RUNTIME/libtcc1.o" "$RUNTIME/va_list.o" "$RUNTIME/alloca86_64.o"

# Native TCC link: executable, shared object, and -l/-L resolution.
"$OUT/bit-tcc" -o "$OUT/hello" "$ROOT/examples/hello.c"
"$OUT/hello" >/dev/null
cat > "$OUT/tcc-lib.c" <<'EOF'
int add(int a, int b) { return a + b; }
EOF
cat > "$OUT/tcc-app.c" <<'EOF'
#include <stdio.h>
int add(int, int);
int main(void) { printf("%d\n", add(20, 22)); return 0; }
EOF
"$OUT/bit-tcc" -shared -o "$OUT/libtccadd.so" "$OUT/tcc-lib.c"
LD_LIBRARY_PATH="$OUT" "$OUT/bit-tcc" -o "$OUT/tcc-app" \
    "$OUT/tcc-app.c" -L"$OUT" -ltccadd
LD_LIBRARY_PATH="$OUT" "$OUT/tcc-app" >/dev/null

"$OUT/bit-tcc" -nostdinc -c "$TCC_SRC/tcc.c" -o "$OUT/tcc-self1.o" \
    -I"$TCC_SRC" -I"$TCC_SRC/include" -I"$INC"
"$OUT/bit-tcc" -nostdinc -c "$TCC_SRC/tcc.c" -o "$OUT/tcc-self2.o" \
    -I"$TCC_SRC" -I"$TCC_SRC/include" -I"$INC"
cmp "$OUT/tcc-self1.o" "$OUT/tcc-self2.o"
"$OUT/bit-tcc" -o "$OUT/tcc-stage2" "$OUT/tcc-self1.o" -ldl -lm
"$OUT/tcc-stage2" -v >/dev/null
echo "PASS: C1 -> TCC -> TCC self-host + TCC native link"
