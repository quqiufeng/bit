#!/bin/sh
# 下载并校验天梯所需的 9 个 GCC 源码包到 $SRC_BASE，可选解包。
# 长期只保留 tarball + SHA256SUMS；解出来的树是派生物，run.sh 会按需重建。
#
#   sh tests/gcc-ladder/fetch.sh              # 下载 + 校验（默认到 /opt/src/gcc）
#   sh tests/gcc-ladder/fetch.sh --extract    # 顺带把 9 棵树解出来（≈7 G）
set -eu

BASE=${BASE:-https://ftp.gnu.org/gnu/gcc}
DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
SRC_BASE=${SRC_BASE:-/opt/src/gcc}
VERSIONS="4.7.4 4.8.5 4.9.4 5.5.0 7.5.0 9.5.0 12.3.0 13.3.0 14.2.0"
EXTRACT=false
[ "${1:-}" = "--extract" ] && EXTRACT=true

mkdir -p "$SRC_BASE"
cd "$SRC_BASE"

# ftp.gnu.org 上 4.x 只有 .tar.bz2，5.x 起是 .tar.xz
get() {
    v=$1
    for e in tar.xz tar.bz2; do
        [ -f "gcc-$v.$e" ] && return 0
        if curl -fsSL --retry 3 -o "gcc-$v.$e.part" "$BASE/gcc-$v/gcc-$v.$e"; then
            mv "gcc-$v.$e.part" "gcc-$v.$e"
            echo "fetched: gcc-$v.$e"
            return 0
        fi
        rm -f "gcc-$v.$e.part"
    done
    echo "FAILED: gcc-$v" >&2
    return 1
}

for v in $VERSIONS; do get "$v"; done

cp "$DIR/SHA256SUMS" ./SHA256SUMS
sha256sum -c SHA256SUMS

if $EXTRACT; then
    for v in $VERSIONS; do
        [ -d "gcc-$v" ] && continue
        for e in tar.xz tar.bz2; do
            [ -f "gcc-$v.$e" ] || continue
            tar xf "gcc-$v.$e"
            echo "extracted: gcc-$v"
            break
        done
    done
fi

du -sh "$SRC_BASE"
