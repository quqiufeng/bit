#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
SRC=${CJSON_SRC:-/opt/cJSON-1.7.18}
CC=${CC:-"$ROOT/build/tcc/bit-tcc"}
OUT=${OUT:-"$ROOT/build/cjson"}
if [ ! -f "$SRC/cJSON.c" ]; then
    echo "SKIP: cJSON source not found: $SRC" >&2
    exit 0
fi
if [ ! -x "$CC" ]; then
    echo "SKIP: bit-tcc not built: $CC" >&2
    exit 0
fi
mkdir -p "$OUT"
"$CC" -c "$SRC/cJSON.c" -I"$SRC" -o "$OUT/cJSON.o"
cat > "$OUT/test.c" <<'EOC'
#include <stdio.h>
#include "cJSON.h"
int main(void) {
    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "n", 42);
    char *s = cJSON_PrintUnformatted(root);
    printf("%s\n", s);
    cJSON_Delete(root);
    return s != 0 ? 0 : 1;
}
EOC
"$CC" -c "$OUT/test.c" -I"$SRC" -o "$OUT/test.o"
"$CC" -o "$OUT/test" "$OUT/test.o" "$OUT/cJSON.o" -lm
"$OUT/test" | grep -qx '{"n":42}'
echo "PASS: bit-tcc compiles and runs cJSON 1.7.18"
