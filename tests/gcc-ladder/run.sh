#!/bin/sh
# TCC -> GCC 4.7 (two passes) -> ... -> GCC 14.2
# All target compilers/libraries are built from build trees; no host compiler
# is used as a target compiler after the initial TCC seed.
set -eu

ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
SRC_BASE=${SRC_BASE:-/opt}
BUILD_BASE=${BUILD_BASE:-/opt/gcc-ladder-build}
TCC=${TCC:-"$ROOT/build/tcc/bit-tcc"}
DEPS=${DEPS:-/opt/gcc47-deps}
JOBS=${JOBS:-2}
WRAP="$BUILD_BASE/wrap"
LOG="$BUILD_BASE/log"
CFLAGS_COMMON="-g -O2 -DLONG_MIN=-9223372036854775807L-1"

need() { [ -e "$1" ] || { echo "missing: $1" >&2; exit 1; }; }
trip() {
    for d in "$1"/*-linux-gnu; do
        case $(basename "$d") in build-*) continue;; esac
        [ -d "$d" ] && { basename "$d"; return; }
    done
    echo "no target triplet under $1" >&2
    exit 1
}

patch_47() {
    src=$1
    for p in "$ROOT/tests/gcc47/cfns-gnu-inline.patch" \
             "$ROOT/tests/gcc-ladder/tcc-dso-handle.patch" \
             "$ROOT/tests/gcc47/tcc-softfp.patch"; do
        [ -f "$p" ] || continue
        patch -d "$src" -p1 --dry-run < "$p" >/dev/null 2>&1 &&
            patch -d "$src" -p1 < "$p"
    done
    f="$src/libgcc/config/i386/linux-unwind.h"
    [ -f "$f" ] && sed -i 's/struct ucontext \*uc_/ucontext_t *uc_/; s/struct ucontext uc;/ucontext_t uc;/' "$f"
}

patch_55() {
    h="$SRC_BASE/gcc-5.5.0/gcc/config/i386/i386.h"
    sed -i 's/const struct stringop_strategy/struct stringop_strategy/; s/const int max;/int max;/; s/const enum stringop_alg alg;/enum stringop_alg alg;/' "$h"
}

make_wrap() {
    prev=$1
    tag=$2
    pt=$(trip "$prev")
    lib="$prev/$pt/libstdc++-v3/src/.libs"
    mkdir -p "$WRAP"
    cat > "$tag-gcc" <<EOF
#!/bin/sh
exec "$prev/gcc/xgcc" -B"$prev/gcc" -L"$lib" "\$@"
EOF
    cat > "$tag-g++" <<EOF
#!/bin/sh
case " \$* " in
  *" -c "*) exec "$prev/gcc/xgcc" -B"$prev/gcc" -L"$lib" -x c++ "\$@" ;;
  *) exec "$prev/gcc/xgcc" -B"$prev/gcc" -L"$lib" "\$@" -lstdc++ ;;
esac
EOF
    chmod +x "$tag-gcc" "$tag-g++"
}

# First pass: TCC compiles GCC 4.7 cc1/cc1plus as C; the resulting cc1plus
# compiles libstdc++.
build_47_pass1() {
    src="$SRC_BASE/gcc-4.7.4"
    d="$BUILD_BASE/4.7-pass1"
    rm -rf "$d"; mkdir -p "$d"; cd "$d"
    CC="$TCC" CXX="$TCC" CC_FOR_BUILD="$TCC" "$src/configure" \
        --disable-bootstrap --enable-languages=c,c++ \
        --disable-multilib --disable-libgomp --disable-libssp \
        --disable-libquadmath --disable-libatomic --disable-decimal-float \
        --disable-werror --prefix="$d/prefix" --with-gmp=/usr \
        --with-mpfr="$DEPS" --with-mpc="$DEPS" >"$LOG/4.7-pass1-conf.log" 2>&1
    make -j"$JOBS" all-gcc CC="$TCC" CXX="$TCC" CC_FOR_BUILD="$TCC" \
        CFLAGS="$CFLAGS_COMMON" CFLAGS_FOR_BUILD="$CFLAGS_COMMON" \
        >"$LOG/4.7-pass1-allgcc.log" 2>&1
    make -j"$JOBS" all-target-libgcc CC="$TCC" CXX="$TCC" CC_FOR_BUILD="$TCC" \
        CFLAGS="$CFLAGS_COMMON" CFLAGS_FOR_BUILD="$CFLAGS_COMMON" \
        >"$LOG/4.7-pass1-libgcc.log" 2>&1
    make -j"$JOBS" all-target-libstdc++-v3 CC="$TCC" CXX="$d/gcc/xgcc" CC_FOR_BUILD="$TCC" \
        >"$LOG/4.7-pass1-libstdcxx.log" 2>&1
    test_cxx 4.7-pass1 "$d" "$src"
}

# Remaining stages: previous GCC build tree builds the next one.
build_stage() {
    name=$1 src=$2 prev=$3 prevsrc=$4 gstd=$5 lstd=$6
    d="$BUILD_BASE/$name"
    tag="$WRAP/$name"
    rm -rf "$d"; mkdir -p "$d"
    make_wrap "$prev" "$tag"
    pt=$(trip "$prev")
    lib="$prev/$pt/libstdc++-v3/src/.libs"
    inc="-I$prev/$pt/libstdc++-v3/include -I$prev/$pt/libstdc++-v3/include/$pt"
    inc="$inc -I$prevsrc/libstdc++-v3/include -I$prevsrc/libstdc++-v3/libsupc++"
    cd "$d"
    CC="$tag-gcc" CXX="$tag-g++" CC_FOR_BUILD="$tag-gcc" "$src/configure" \
        --disable-bootstrap --enable-languages=c,c++ \
        --disable-multilib --disable-libgomp --disable-libssp \
        --disable-libquadmath --disable-libatomic --disable-decimal-float \
        --disable-werror --prefix="$d/prefix" --with-gmp=/usr \
        --with-mpfr="$DEPS" --with-mpc="$DEPS" >"$LOG/$name-conf.log" 2>&1
    make -j"$JOBS" all-gcc CC="$tag-gcc" CXX="$tag-g++" CC_FOR_BUILD="$tag-gcc" \
        CFLAGS="$CFLAGS_COMMON" CFLAGS_FOR_BUILD="$CFLAGS_COMMON" \
        CXXFLAGS="-g -O2 -std=$gstd $inc" LDFLAGS="-L$lib" \
        >"$LOG/$name-allgcc.log" 2>&1
    make -j"$JOBS" all-target-libgcc CC="$tag-gcc" CXX="$tag-g++" CC_FOR_BUILD="$tag-gcc" \
        >"$LOG/$name-libgcc.log" 2>&1
    make -j"$JOBS" all-target-libstdc++-v3 CC="$tag-gcc" CXX="$tag-g++" CC_FOR_BUILD="$tag-gcc" \
        CXXFLAGS="-g -O2 -std=$lstd" LDFLAGS="-L$lib" \
        >"$LOG/$name-libstdcxx.log" 2>&1
    test_cxx "$name" "$d" "$src"
}

test_cxx() {
    name=$1 d=$2 src=$3
    t=$(trip "$d")
    Inc="-I$d/$t/libstdc++-v3/include -I$d/$t/libstdc++-v3/include/$t -I$src/libstdc++-v3/include -I$src/libstdc++-v3/libsupc++"
    cat > "$d/hello.cc" <<'EOC'
#include <iostream>
int main(){std::cout<<"OK "<<__VERSION__<<"\n";return 0;}
EOC
    "$d/gcc/xgcc" -B"$d/gcc" -x c++ $Inc "$d/hello.cc" -o "$d/hello" \
        -L"$d/$t/libstdc++-v3/src/.libs" -lstdc++ \
        >"$LOG/$name-hello.log" 2>&1
    LD_LIBRARY_PATH="$d/$t/libstdc++-v3/src/.libs:$DEPS/lib" "$d/hello" \
        >"$LOG/$name-hello.out"
    echo "PASS: $name"
}

main() {
    need "$TCC"; need "$DEPS/include/mpfr.h"
    mkdir -p "$BUILD_BASE" "$LOG"
    export LD_LIBRARY_PATH="$DEPS/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
    patch_47 "$SRC_BASE/gcc-4.7.4"
    [ -f "$LOG/4.7-pass1-hello.out" ] || build_47_pass1
    [ -f "$LOG/4.7-pass2-hello.out" ] || build_stage 4.7-pass2 "$SRC_BASE/gcc-4.7.4" "$BUILD_BASE/4.7-pass1" "$SRC_BASE/gcc-4.7.4" gnu++98 gnu++11
    [ -f "$LOG/4.8-hello.out" ] || build_stage 4.8 "$SRC_BASE/gcc-4.8.5" "$BUILD_BASE/4.7-pass2" "$SRC_BASE/gcc-4.7.4" gnu++98 gnu++11
    [ -f "$LOG/4.9-hello.out" ] || build_stage 4.9 "$SRC_BASE/gcc-4.9.4" "$BUILD_BASE/4.8" "$SRC_BASE/gcc-4.8.5" gnu++11 gnu++11
    [ -f "$LOG/5.5-hello.out" ] || { patch_55; build_stage 5.5 "$SRC_BASE/gcc-5.5.0" "$BUILD_BASE/4.9" "$SRC_BASE/gcc-4.9.4" gnu++11 gnu++11; }
    [ -f "$LOG/7.5-hello.out" ] || build_stage 7.5 "$SRC_BASE/gcc-7.5.0" "$BUILD_BASE/5.5" "$SRC_BASE/gcc-5.5.0" gnu++11 gnu++11
    [ -f "$LOG/9.5-hello.out" ] || build_stage 9.5 "$SRC_BASE/gcc-9.5.0" "$BUILD_BASE/7.5" "$SRC_BASE/gcc-7.5.0" gnu++11 gnu++11
    [ -f "$LOG/12.3-hello.out" ] || build_stage 12.3 "$SRC_BASE/gcc-12.3.0" "$BUILD_BASE/9.5" "$SRC_BASE/gcc-9.5.0" gnu++11 gnu++11
    [ -f "$LOG/13.3-hello.out" ] || build_stage 13.3 "$SRC_BASE/gcc-13.3.0" "$BUILD_BASE/12.3" "$SRC_BASE/gcc-12.3.0" gnu++11 gnu++11
    [ -f "$LOG/14.2-hello.out" ] || build_stage 14.2 "$SRC_BASE/gcc-14.2.0" "$BUILD_BASE/13.3" "$SRC_BASE/gcc-13.3.0" gnu++11 gnu++11
    echo "PASS: TCC -> GCC 4.7 (2 passes) -> GCC 14.2"
}

main "$@"
