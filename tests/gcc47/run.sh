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
GCC47_BUILD=${GCC47_BUILD:-/opt/gcc47-build/gcc}
if [ -f "$GCC47_BUILD/gcc/gtype-desc.h" ] && [ -f "$GCC47_BUILD/gcc/bversion.h" ]; then
    rm -rf "$OUT/gcc-infra"
    mkdir -p "$OUT/gcc-infra"
    cd "$OUT/gcc-infra"
    for f in ggc-page.c bitmap.c vec.c double-int.c tree.c diagnostic.c; do
        "$CC" -DIN_GCC -DHAVE_CONFIG_H -I"$GCC47_BUILD/gcc" \
            -I"$SRC/gcc" -I"$SRC/include" -I"$SRC/libcpp/include" \
            -I"$SRC/libiberty" -c "$SRC/gcc/$f" -o "${f%.c}.o"
    done
    for f in i386.c i386-c.c; do
        "$CC" -DIN_GCC -DHAVE_CONFIG_H -I"$GCC47_BUILD/gcc" \
            -I"$SRC/gcc" -I"$SRC/gcc/config" -I"$SRC/include" \
            -I"$SRC/libcpp/include" -I"$SRC/libiberty" \
            -c "$SRC/gcc/config/i386/$f" -o "${f%.c}.o"
    done
    for f in c-decl.c c-parser.c c-typeck.c c-convert.c c-lang.c; do
        "$CC" -DIN_GCC -DHAVE_CONFIG_H -I"$GCC47_BUILD/gcc" \
            -I"$SRC/gcc" -I"$SRC/gcc/config" -I"$SRC/gcc/c-family" \
            -I"$SRC/include" -I"$SRC/libcpp/include" -I"$SRC/libiberty" \
            -c "$SRC/gcc/$f" -o "${f%.c}.o"
    done
    for f in c-common.c c-format.c c-lex.c c-cppbuiltin.c c-opts.c \
             c-pragma.c c-semantics.c; do
        "$CC" -DIN_GCC -DHAVE_CONFIG_H -I"$GCC47_BUILD/gcc" \
            -I"$SRC/gcc" -I"$SRC/gcc/config" -I"$SRC/gcc/c-family" \
            -I"$SRC/include" -I"$SRC/libcpp/include" -I"$SRC/libiberty" \
            -c "$SRC/gcc/c-family/$f" -o "${f%.c}.o"
    done
    echo "PASS: bit-tcc compiles GCC 4.7.4 libiberty, libcpp, gcc infra, x86 backend and C frontend probes"
else
    echo "PASS: bit-tcc compiles GCC 4.7.4 libiberty and libcpp probes (infra skipped)"
fi
