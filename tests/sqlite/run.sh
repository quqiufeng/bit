#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
SRC=${SQLITE_SRC:-/opt/sqlite-amalgamation-3450300}
CC=${CC:-"$ROOT/build/tcc/bit-tcc"}
OUT=${OUT:-"$ROOT/build/sqlite"}
if [ ! -f "$SRC/sqlite3.c" ]; then
    echo "SKIP: SQLite source not found: $SRC" >&2
    exit 0
fi
if [ ! -x "$CC" ]; then
    echo "SKIP: bit-tcc not built: $CC" >&2
    exit 0
fi
mkdir -p "$OUT"
"$CC" -c "$SRC/sqlite3.c" -I"$SRC" -o "$OUT/sqlite3.o"
cat > "$OUT/test.c" <<'EOC'
#include <stdio.h>
#include "sqlite3.h"
int main(void) {
    sqlite3 *db = 0;
    sqlite3_stmt *st = 0;
    if (sqlite3_open(":memory:", &db) != SQLITE_OK) return 1;
    if (sqlite3_exec(db, "create table t(x); insert into t values(42);", 0, 0, 0) != SQLITE_OK) return 2;
    if (sqlite3_prepare_v2(db, "select x from t", -1, &st, 0) != SQLITE_OK) return 3;
    if (sqlite3_step(st) != SQLITE_ROW) return 4;
    printf("%d\n", sqlite3_column_int(st, 0));
    sqlite3_finalize(st);
    sqlite3_close(db);
    return 0;
}
EOC
"$CC" -c "$OUT/test.c" -I"$SRC" -o "$OUT/test.o"
"$CC" -o "$OUT/test" "$OUT/test.o" "$OUT/sqlite3.o" -lpthread -ldl -lm
"$OUT/test" | grep -qx 42
echo "PASS: bit-tcc compiles and runs SQLite 3.45.3"
