#!/bin/sh
# 为每个已完成的阶梯级写 witness 与 MANIFEST：
#   log/<name>-witness.txt   xgcc --version、hello 输出、拿本级编译器现编 C/C++ 并运行
#   log/<name>-MANIFEST      cc1/cc1plus/xgcc/libstdc++ 的尺寸与 sha256
# 用法: witness.sh <build_base> [name src ...]   (不给 name = 扫全部 <name>-hello.out)
set -eu

BUILD_BASE=${1:?usage: witness.sh <build_base> [name...]}
shift
LOG="$BUILD_BASE/log"
DEPS=${DEPS:-/opt/gcc47-deps}

# 参数成对: name src（src = 该级 GCC 源码树，用来补 libstdc++ 的 <exception> 等公共头）
set -- "$@"
pairs=$*
if [ -z "$pairs" ]; then
    for f in "$LOG"/*-hello.out; do
        pairs="$pairs $(basename "$f" -hello.out) -"
    done
fi

triplet() {
    for d in "$1"/*-linux-gnu; do
        case $(basename "$d") in build-*) continue;; esac
        [ -d "$d" ] && { basename "$d"; return; }
    done
    echo unknown
}

set -- $pairs
while [ $# -gt 0 ]; do
    name=$1; src=${2:--}; shift; [ $# -gt 0 ] && shift
    d="$BUILD_BASE/$name"
    [ -x "$d/gcc/xgcc" ] || { echo "skip $name (no xgcc)" >&2; continue; }
    t=$(triplet "$d")
    lib="$d/$t/libstdc++-v3/src/.libs"
    sha() { sha256sum "$1" 2>/dev/null | cut -d' ' -f1; }
    size() { stat -c%s "$1" 2>/dev/null || echo -; }

    w="$LOG/$name-witness.txt"
    {
        echo "# witness: $name"
        echo "host: $(uname -sr) $(uname -m)   date: $(date -Is)"
        echo
        echo "== xgcc --version"
        "$d/gcc/xgcc" -B"$d/gcc" --version 2>&1 | head -1
        echo
        echo "== 用本级编译器现编 C（不碰宿主 gcc）"
        printf '#include <stdio.h>\nint main(void){printf("%%s %%d\\n", __VERSION__, (int)sizeof(void*));return 0;}\n' > "$d/w.c"
        "$d/gcc/xgcc" -B"$d/gcc" "$d/w.c" -o "$d/w-c" && "$d/w-c"
        echo "  c-bin sha256: $(sha "$d/w-c")"
        echo
        echo "== 用本级编译器现编 C++（本级 libstdc++）"
        printf '#include <iostream>\nint main(){std::cout<<"cxx-ok "<<__VERSION__<<"\\n";return 0;}\n' > "$d/w.cc"
        LD_LIBRARY_PATH="$lib:$DEPS/lib" "$d/gcc/xgcc" -B"$d/gcc" -x c++ \
            -I"$d/$t/libstdc++-v3/include" -I"$d/$t/libstdc++-v3/include/$t" \
            -I"$src/libstdc++-v3/include" -I"$src/libstdc++-v3/libsupc++" \
            -L"$lib" "$d/w.cc" -o "$d/w-cxx" -lstdc++ &&
            LD_LIBRARY_PATH="$lib:$DEPS/lib" "$d/w-cxx"
        echo "  cxx-bin sha256: $(sha "$d/w-cxx")"
        echo
        echo "== hello（阶梯自带 test_cxx 产物）"
        cat "$LOG/$name-hello.out" 2>/dev/null
    } > "$w" 2>&1

    m="$LOG/$name-MANIFEST"
    {
        echo "# MANIFEST: $name   ($(date -Is))"
        echo "triplet: $t"
        for f in gcc/cc1 gcc/cc1plus gcc/xgcc gcc/collect2 "$t/libstdc++-v3/src/.libs/libstdc++.so"; do
            p="$d/$f"
            if [ -e "$p" ]; then
                # libstdc++.so 是符号链接，跟到真身
                real=$(readlink -f "$p")
                printf '%-46s %10s  %s\n' "$f" "$(size "$real")" "$(sha "$real")"
            else
                printf '%-46s %10s  %s\n' "$f" - -
            fi
        done
    } > "$m"
    echo "witness: $name -> $m"
done
