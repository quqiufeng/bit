#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
SRC=${GCC47_SRC:-/opt/gcc-4.7.4}
CC=${CC:-"$ROOT/build/tcc/bit-tcc"}
OUT=${OUT:-"$ROOT/build/gcc47"}
if [ ! -f "$SRC/libiberty/cplus-dem.c" ]; then
    echo "SKIP: GCC 4.7.4 source not found: $SRC" >&2
    exit 0
fi
if [ ! -x "$CC" ]; then
    echo "SKIP: bit-tcc not built: $CC" >&2
    exit 0
fi
rm -rf "$OUT/libiberty"
mkdir -p "$OUT/libiberty"
cd "$OUT/libiberty"
CC=gcc "$SRC/libiberty/configure" --enable-install-libiberty >/tmp/bit-gcc47-libiberty-conf.log 2>&1
for f in cplus-dem.c argv.c obstack.c hashtab.c splay-tree.c fibheap.c pex-unix.c; do
    "$CC" -DHAVE_CONFIG_H -I. -I"$SRC/libiberty" -I"$SRC/include" \
        -c "$SRC/libiberty/$f" -o "${f%.c}.o"
done

rm -rf "$OUT/libcpp"
mkdir -p "$OUT/libcpp"
cd "$OUT/libcpp"
CC=gcc "$SRC/libcpp/configure" >/tmp/bit-gcc47-libcpp-conf.log 2>&1
printf '#define LOCALEDIR "/usr/share/locale"\n' > localedir.h
for f in lex.c macro.c directives.c expr.c files.c identifiers.c init.c \
         line-map.c symtab.c charset.c traditional.c pch.c mkdeps.c errors.c; do
    "$CC" -DHAVE_CONFIG_H -I. -I"$SRC/libcpp" -I"$SRC/libcpp/include" \
        -I"$SRC/include" -I"$SRC/libiberty" \
        -c "$SRC/libcpp/$f" -o "${f%.c}.o"
done
echo "PASS: bit-tcc compiles GCC 4.7.4 libiberty and libcpp probes"
