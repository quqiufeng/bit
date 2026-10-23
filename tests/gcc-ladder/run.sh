#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
SRC_BASE=${SRC_BASE:-/opt}
BUILD_BASE=${BUILD_BASE:-/opt/gcc-ladder-build}
PREFIX_BASE=${PREFIX_BASE:-/opt/gcc-ladder}
TCC=${TCC:-"$ROOT/build/tcc/bit-tcc"}
DEPS=${DEPS:-/opt/gcc47-deps}
HOST_CC=${HOST_CC:-gcc}
HOST_CXX=${HOST_CXX:-g++}
SYSGCC=${SYSGCC:-/usr/lib/gcc/x86_64-linux-gnu/13}
STAGES=${STAGES:-"4.7 4.8 4.9 5.5 7.5 9.5 12.3 13.3 14.2"}

need() { [ -e "$1" ] || { echo "missing: $1" >&2; exit 1; }; }

patch_ucontext() {
    src=$1
    f="$src/libgcc/config/i386/linux-unwind.h"
    if [ -f "$f" ]; then
        sed -i 's/struct ucontext \*uc_/ucontext_t *uc_/; s/struct ucontext uc;/ucontext_t uc;/' "$f"
    fi
}

apply_47_patch() {
    src=$1
    p="$ROOT/tests/gcc47/cfns-gnu-inline.patch"
    if patch -d "$src" -p1 --dry-run < "$p" >/dev/null 2>&1; then
        patch -d "$src" -p1 < "$p"
    fi
    p="$ROOT/tests/gcc-ladder/tcc-dso-handle.patch"
    if patch -d "$src" -p1 --dry-run < "$p" >/dev/null 2>&1; then
        patch -d "$src" -p1 < "$p"
    fi
}

configure_and_build() {
    version=$1
    src=$2
    prev=$3
    gcc_flags=$4
    lib_flags=$5
    build="$BUILD_BASE/$version/gcc"
    prefix="$PREFIX_BASE/$version"
    rm -rf "$build"
    mkdir -p "$build"
    cd "$build"

    if [ "$version" = "4.7" ]; then
        CC="$HOST_CC" CXX="$HOST_CXX" "$src/configure" \
            --disable-bootstrap --enable-languages=c,c++ \
            --disable-multilib --disable-libgomp --disable-libssp \
            --disable-libquadmath --disable-libatomic --disable-decimal-float --disable-werror \
            --prefix="$prefix" --with-gmp=/usr \
            --with-mpfr="$DEPS" --with-mpc="$DEPS" >/tmp/gcc-ladder-$version-conf.log 2>&1
        make_cc="$TCC"
        make_cxx="$HOST_CXX"
        build_cc="$HOST_CC"
    else
        need "$prev/bin/gcc"
        need "$prev/bin/g++"
        CC="$prev/bin/gcc" CXX="$prev/bin/g++" "$src/configure" \
            --disable-bootstrap --enable-languages=c,c++ \
            --disable-multilib --disable-libgomp --disable-libssp \
            --disable-libquadmath --disable-libatomic --disable-decimal-float --disable-werror \
            --prefix="$prefix" --with-gmp=/usr \
            --with-mpfr="$DEPS" --with-mpc="$DEPS" >/tmp/gcc-ladder-$version-conf.log 2>&1
        make_cc="$prev/bin/gcc"
        make_cxx="$prev/bin/g++"
        build_cc="$prev/bin/gcc"
    fi

    export LD_LIBRARY_PATH="$DEPS/lib:$prev/lib64:$prev/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
    if [ "$version" = "4.7" ]; then
        # First build GCC/support libraries with host tools, then rebuild all
        # gcc objects with TCC while C++ sources remain on the host C++ bridge.
        make -j2 all-gcc CC="$HOST_CC" CXX="$HOST_CXX" CC_FOR_BUILD="$HOST_CC" \
            CXXFLAGS="$gcc_flags" >/tmp/gcc-ladder-4.7-host-bootstrap.log 2>&1
        find gcc -name '*.o' -delete
        rm -f gcc/libbackend.a gcc/libcommon.a gcc/libcommon-target.a
    fi
    make -j2 all-gcc \
        CC="$make_cc" CXX="$make_cxx" CC_FOR_BUILD="$build_cc" \
        CXXFLAGS="$gcc_flags" >/tmp/gcc-ladder-$version-allgcc.log 2>&1
    if [ "$version" != "4.7" ]; then
        make -j2 all-target-libgcc \
            CC="$make_cc" CXX="$make_cxx" CC_FOR_BUILD="$build_cc" \
            CXXFLAGS="$gcc_flags" >/tmp/gcc-ladder-$version-libgcc.log 2>&1
    else
        echo "SKIP: GCC 4.7 libgcc; use host libgcc for the C++ bridge" >/tmp/gcc-ladder-4.7-libgcc.log
    fi
    make -j2 all-target-libstdc++-v3 \
        CC="$make_cc" CXX="$make_cxx" CC_FOR_BUILD="$build_cc" \
        CXXFLAGS="$lib_flags" >/tmp/gcc-ladder-$version-libstdcxx.log 2>&1
    make install \
        CC="$make_cc" CXX="$make_cxx" CC_FOR_BUILD="$build_cc" \
        CXXFLAGS="$lib_flags" >/tmp/gcc-ladder-$version-install.log 2>&1

    cat > "$build/hello.c" <<'EOC'
int main(void){return 42;}
EOC
    cat > "$build/hello.cc" <<'EOC'
#include <iostream>
int main(){std::cout<<"gcc"<<' '<<42<<"\n";return 0;}
EOC
    LD_LIBRARY_PATH="$DEPS/lib:$prefix/lib64:$prefix/lib" "$prefix/bin/gcc" -o "$build/hello-c" "$build/hello.c"
    LD_LIBRARY_PATH="$DEPS/lib:$prefix/lib64:$prefix/lib" "$prefix/bin/g++" -o "$build/hello-cpp" "$build/hello.cc"
    "$build/hello-c" || { [ $? -eq 42 ] || exit 1; }
    LD_LIBRARY_PATH="$DEPS/lib:$prefix/lib64:$prefix/lib" "$build/hello-cpp" >/dev/null
    echo "PASS: GCC $version ladder installed"
}

main() {
    need "$TCC"
    need "$DEPS/include/mpfr.h"
    for v in $STAGES; do
        case "$v" in
            4.7)
                src="$SRC_BASE/gcc-4.7.4"
                need "$src/libiberty/cplus-dem.c"
                apply_47_patch "$src"
                patch_ucontext "$src"
                configure_and_build 4.7 "$src" "" "-g -O2 -std=gnu++98" "-g -O2 -std=gnu++11"
                ;;
            4.8)
                src="$SRC_BASE/gcc-4.8.5"
                patch_ucontext "$src"
                configure_and_build 4.8 "$src" "$PREFIX_BASE/4.7" "-g -O2 -std=gnu++98" "-g -O2 -std=gnu++11"
                ;;
            4.9)
                src="$SRC_BASE/gcc-4.9.4"
                patch_ucontext "$src"
                configure_and_build 4.9 "$src" "$PREFIX_BASE/4.8" "-g -O2 -std=gnu++98" "-g -O2 -std=gnu++11"
                ;;
            5.5)
                src="$SRC_BASE/gcc-5.5.0"
                patch_ucontext "$src"
                configure_and_build 5.5 "$src" "$PREFIX_BASE/4.9" "-g -O2 -std=gnu++98" "-g -O2 -std=gnu++11"
                ;;
            7.5)
                src="$SRC_BASE/gcc-7.5.0"
                patch_ucontext "$src"
                configure_and_build 7.5 "$src" "$PREFIX_BASE/5.5" "-g -O2 -std=gnu++11" "-g -O2 -std=gnu++17"
                ;;
            9.5)
                src="$SRC_BASE/gcc-9.5.0"
                patch_ucontext "$src"
                configure_and_build 9.5 "$src" "$PREFIX_BASE/7.5" "-g -O2 -std=gnu++14" "-g -O2 -std=gnu++17"
                ;;
            12.3)
                src="$SRC_BASE/gcc-12.3.0"
                patch_ucontext "$src"
                configure_and_build 12.3 "$src" "$PREFIX_BASE/9.5" "-g -O2 -std=gnu++11" "-g -O2 -std=gnu++17"
                ;;
            13.3)
                src="$SRC_BASE/gcc-13.3.0"
                patch_ucontext "$src"
                configure_and_build 13.3 "$src" "$PREFIX_BASE/12.3" "-g -O2 -std=gnu++11" "-g -O2 -std=gnu++17"
                ;;
            14.2)
                src="$SRC_BASE/gcc-14.2.0"
                patch_ucontext "$src"
                configure_and_build 14.2 "$src" "$PREFIX_BASE/13.3" "-g -O2 -std=gnu++11" "-g -O2 -std=gnu++17"
                ;;
            *) echo "unknown stage: $v" >&2; exit 1;;
        esac
    done
}
main
