#!/bin/sh
# 把阶梯构建树裁剪成「只留可执行件」的运行时树，并逐级验证它还能编译 C/C++。
#   构建树: 19 GB（含 .o/.a/中间库/544 MB 的 libstdc++ .gch）
#   裁剪后: 只剩 cc1/cc1plus/lto*/xgcc/collect2 + crt*.o + libgcc*.a + libstdc++.so + 头文件
#
# 用法: prune.sh [build_base] [out_base]
#   默认 /opt/gcc-ladder-2026 -> /opt/gcc-ladder-bin
# 产物: <out>/<stage>/gcc/...  <out>/<stage>/lib/...  <out>/<stage>/include/c++/<ver>/...
#       <out>/bin/<stage>-gcc <out>/bin/<stage>-g++  (带 -B/-I/-L/-rpath 的包装)
#       <out>/log/<stage>-prune.txt  (版本、尺寸、sha256、C/C++ 现编结果)
set -eu

BUILD_BASE=${1:-/opt/gcc-ladder-2026}
OUT=${2:-/opt/gcc-ladder-bin}
SRC_BASE=${SRC_BASE:-/opt/src/gcc}
LOG="$OUT/log"
HDR_CACHE="$OUT/.hdr"

# stage:源码版本
STAGES=${STAGES:-"4.7-pass1:4.7.4 4.7-pass2:4.7.4 4.8:4.8.5 4.9:4.9.4 5.5:5.5.0 7.5:7.5.0 9.5:9.5.0 12.3:12.3.0 13.3:13.3.0 14.2:14.2.0"}

trip() {
    for d in "$1"/*-linux-gnu; do
        case $(basename "$d") in build-*) continue;; esac
        [ -d "$d" ] && { basename "$d"; return; }
    done
    echo unknown
}

is_elf() { [ -f "$1" ] && head -c4 "$1" | od -An -c | grep -q 'E   L   F'; }

# 需要保留的可执行件/链接件（相对 gcc/）
BINS="cc1 cc1plus lto1 lto-dump xgcc collect2 lto-wrapper liblto_plugin.so gcc-ar gcc-nm gcc-ranlib"

# 从 tarball 里补出 libstdc++ 公共头（构建树里的 include/ 只是指向源码树的符号链接，
# 源码树按约定在跑完后已删除；.gch 预编译头 544 MB，属于缓存，直接扔掉）
materialize_cxx_headers() {
    stage=$1 v=$2 d=$3
    dst="$d/include/c++/$v"
    srcc="$HDR_CACHE/src/gcc-$v"
    bt=$(trip "$BUILD_BASE/$stage")
    srcInc="$BUILD_BASE/$stage/$bt/libstdc++-v3/include"
    # 公共头 + config/ 模板：构建树里大量头是指向源码树的符号链接，而源码树按约定
    # 跑完即删，所以真身从 tarball 再取一份（只解 include/ libsupc++/ config/，很小）。
    if [ ! -d "$srcc/libstdc++-v3/include" ]; then
        t=
        for e in tar.xz tar.bz2; do [ -f "$SRC_BASE/gcc-$v.$e" ] && { t="$SRC_BASE/gcc-$v.$e"; break; }; done
        [ -n "$t" ] || { echo "  ! missing tarball for $v" >&2; return 1; }
        mkdir -p "$HDR_CACHE/src"
        tar xf "$t" -C "$HDR_CACHE/src" "gcc-$v/libstdc++-v3/include" \
            "gcc-$v/libstdc++-v3/libsupc++" "gcc-$v/libstdc++-v3/config" 2>/dev/null ||
            tar xf "$t" -C "$HDR_CACHE/src" "gcc-$v/libstdc++-v3/include"
    fi
    if [ ! -d "$dst/bits" ]; then
        mkdir -p "$dst"
        cp -a "$srcc/libstdc++-v3/include/." "$dst/"
        [ -d "$srcc/libstdc++-v3/libsupc++" ] && cp -a "$srcc/libstdc++-v3/libsupc++" "$dst/libsupc++"
    fi
    # 生成的 target 头（bits/c++config.h、gthr*.h…）是真文件，直接拷；.gch 是 544 MB 预编译缓存，丢
    if [ -d "$srcInc/$bt" ]; then
        ( cd "$srcInc" && find "$bt" -name '*.gch' -prune -o -type f -print0 |
          tar --null -T - -cf - | tar xf - -C "$dst" ) || true
    fi
    # 其余头在 build 树里是指向源码树的链接：按同一相对路径把真身落地
    n=0; miss=0
    for l in $(cd "$srcInc" && find . -name '*.gch' -prune -o -type l -print | sed 's|^\./||'); do
        tgt=$(readlink "$srcInc/$l")
        rel=${tgt#"$SRC_BASE/gcc-$v/libstdc++-v3/"}
        if [ "$rel" != "$tgt" ] && [ -f "$srcc/libstdc++-v3/$rel" ]; then
            mkdir -p "$dst/$(dirname "$l")"
            cp "$srcc/libstdc++-v3/$rel" "$dst/$l"; n=$((n+1))
        else
            miss=$((miss+1))
        fi
    done
    echo "  头: 链接落地 $n 个, 未解 $miss 个"
    [ -d "$dst/bits" ] && return 0
    echo "  ! libstdc++ headers incomplete for $v" >&2
    return 1
}

prune_one() {
    stage=$1 v=$2
    src="$BUILD_BASE/$stage"
    d="$OUT/$stage"
    bt=$(trip "$src")
    [ -x "$src/gcc/cc1" ] || { echo "skip $stage (no cc1)" >&2; return 0; }
    echo "== $stage (源码 $v, target $bt)"

    rm -rf "$d"
    mkdir -p "$d/gcc" "$d/lib" "$d/bin" "$OUT/bin"

    # 1) 编译器可执行件
    for b in $BINS; do
        [ -e "$src/gcc/$b" ] || continue
        cp -aL "$src/gcc/$b" "$d/gcc/$b"   # 老版本的 gcc/liblto_plugin.so 是指向 lto-plugin/.libs 的链接，落地成真身
        is_elf "$d/gcc/$b" && strip "$d/gcc/$b" 2>/dev/null || true
    done
    # 2) 链接必需的小件
    for f in "$src"/gcc/crt*.o "$src"/gcc/libgcc.a "$src"/gcc/libgcc_eh.a "$src"/gcc/libgcov.a; do
        [ -f "$f" ] && cp -a "$f" "$d/gcc/"
    done
    for f in "$src/$bt"/libgcc/libgcc.a "$src/$bt"/libgcc/libgcc_eh.a "$src/$bt"/libgcc/libgcov.a; do
        [ -f "$f" ] && cp -a "$f" "$d/lib/"
    done
    # libgcc_s：-lgcc_s 在链接期就要能找到（静态 .a 之外还得有共享版真身）
    for f in "$src/$bt"/libgcc/libgcc_s.so "$src/$bt"/libgcc/libgcc_s.so.1; do
        [ -e "$f" ] || continue
        cp -a "$f" "$d/lib/"
        cp -a "$f" "$d/gcc/"
    done
    # 3) 头
    cp -a "$src/gcc/include" "$d/gcc/include"
    [ -d "$src/gcc/include-fixed" ] && cp -a "$src/gcc/include-fixed" "$d/gcc/include-fixed"
    materialize_cxx_headers "$stage" "$v" "$d" || true
    # 4) libstdc++.so（跟到真身，重建 .so/.so.6 两个名字）
    so=$(ls "$src/$bt"/libstdc++-v3/src/.libs/libstdc++.so.6.* 2>/dev/null | head -1)
    if [ -n "$so" ]; then
        real=$(basename "$(readlink -f "$so")")
        cp -a "$(readlink -f "$so")" "$d/lib/$real"
        ln -sf "$real" "$d/lib/libstdc++.so.6"
        ln -sf "$real" "$d/lib/libstdc++.so"
    fi

    # 5) 包装：-B/-I/-L/-rpath 一次配好
    cat > "$d/bin/$stage-gcc" <<EOF
#!/bin/sh
D=$d
exec "\$D/gcc/xgcc" -B"\$D/gcc" -I"\$D/include/c++/$v" -I"\$D/include/c++/$v/$bt" \\
    -I"\$D/include/c++/$v/libsupc++" -L"\$D/lib" "\$@"
EOF
    cat > "$d/bin/$stage-g++" <<EOF
#!/bin/sh
D=$d
export LD_LIBRARY_PATH="\$D/lib\${LD_LIBRARY_PATH:+:\$LD_LIBRARY_PATH}"
exec "\$D/gcc/xgcc" -B"\$D/gcc" -x c++ -I"\$D/include/c++/$v" -I"\$D/include/c++/$v/$bt" \\
    -I"\$D/include/c++/$v/libsupc++" -I"\$D/include/c++/$v/backward" -L"\$D/lib" \\
    -Wl,-rpath,"\$D/lib" "\$@" -lstdc++
EOF
    chmod +x "$d/bin/$stage-gcc" "$d/bin/$stage-g++"
    ln -sf "$d/bin/$stage-gcc" "$OUT/bin/$stage-gcc"
    ln -sf "$d/bin/$stage-g++" "$OUT/bin/$stage-g++"

    # 6) 现编现跑（不许借宿主 gcc；C++ 走本级 libstdc++）
    mkdir -p "$LOG"
    w="$LOG/$stage-prune.txt"
    printf '#include <stdio.h>\nint main(void){printf("c-ok %%s %%d\\n",__VERSION__,(int)sizeof(void*));return 0;}\n' > "$d/smoke.c"
    printf '#include <iostream>\nint main(){std::cout<<"cxx-ok "<<__VERSION__<<"\\n";return 0;}\n' > "$d/smoke.cc"
    {
        echo "# PRUNE: $stage  (源码 $v, target $bt)  $(date -Is)"
        echo "== xgcc --version"
        "$d/gcc/xgcc" -B"$d/gcc" --version 2>&1 | head -1
        echo "== C 现编现跑"
        "$d/bin/$stage-gcc" "$d/smoke.c" -o "$d/smoke-c" 2>&1 && "$d/smoke-c"
        echo "== C++ 现编现跑"
        "$d/bin/$stage-g++" "$d/smoke.cc" -o "$d/smoke-cxx" 2>&1 && "$d/smoke-cxx"
        echo "== 尺寸/sha256"
        for f in gcc/cc1 gcc/cc1plus gcc/xgcc gcc/collect2 lib/libstdc++.so.6; do
            printf '%-24s %10s  %s\n' "$f" "$(stat -c%s "$d/$f" 2>/dev/null || echo -)" \
                "$(sha256sum "$d/$f" 2>/dev/null | cut -d' ' -f1)"
        done
        echo "== 树大小: $(du -sh "$d" | cut -f1)"
        echo "== 头文件数: $(find "$d/include" -type f 2>/dev/null | wc -l)   gch: $(find "$d/include" -name '*.gch' 2>/dev/null | wc -l)"
    } > "$w" 2>&1
    rm -f "$d/smoke.c" "$d/smoke.cc"
    # smoke 产物留着当证据
    if "$d/smoke-c" >/dev/null 2>&1 && "$d/smoke-cxx" >/dev/null 2>&1; then
        echo "  PASS: $stage -> $w  ($(du -sh "$d" | cut -f1))"
    else
        echo "  FAIL: $stage 见 $w" >&2
    fi
}

for sv in $STAGES; do
    prune_one "${sv%%:*}" "${sv##*:}"
done

echo
echo "== 汇总"
du -sh "$OUT"
for d in "$OUT"/*/; do
    case $d in */log/|*/.hdr/) continue;; esac
    printf '  %-12s %s\n' "$(basename "$d")" "$(du -sh "$d" | cut -f1)"
done
