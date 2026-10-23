#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
SRC=${LUA_SRC:-/opt/lua-5.4.7}
CC=${CC:-"$ROOT/build/tcc/bit-tcc"}
OUT=${OUT:-"$ROOT/build/lua"}
if [ ! -f "$SRC/Makefile" ]; then
    echo "SKIP: Lua source not found: $SRC" >&2
    exit 0
fi
if [ ! -x "$CC" ]; then
    echo "SKIP: bit-tcc not built: $CC" >&2
    exit 0
fi
rm -rf "$OUT"
mkdir -p "$OUT"
cp -a "$SRC/." "$OUT/"
cd "$OUT"
make posix CC="$CC" >/tmp/bit-lua-build.log 2>&1 || {
    echo "FAIL: Lua build; see /tmp/bit-lua-build.log" >&2
    exit 1
}
src/lua -e 'assert(6*7 == 42); print("lua ok")' | grep -qx 'lua ok'
echo "PASS: bit-tcc compiles and runs Lua 5.4.7"
