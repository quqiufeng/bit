#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
BUSYBOX_SRC=${BUSYBOX_SRC:-/tmp/busybox-1.35.0}
CC=${CC:-"$ROOT/build/tcc/bit-tcc"}
OUT=${OUT:-"$ROOT/build/tcc/busybox"}
if [ ! -f "$BUSYBOX_SRC/Makefile" ]; then
    echo "SKIP: BusyBox source not found: $BUSYBOX_SRC" >&2
    exit 0
fi
if [ ! -x "$CC" ]; then
    echo "SKIP: bit-tcc not built: $CC" >&2
    exit 0
fi
rm -rf "$OUT"
mkdir -p "$OUT"
cp -a "$BUSYBOX_SRC/." "$OUT/"
cd "$OUT"
make allnoconfig >/dev/null
for applet in CAT ECHO LS TRUE FALSE; do
    sed -i "s/^# CONFIG_${applet} is not set/CONFIG_${applet}=y/" .config
done
yes '' | make oldconfig >/dev/null
make clean >/dev/null 2>&1 || true
make -j2 CC="$CC" HOSTCC=gcc SKIP_STRIP=y >/tmp/bit-busybox-build.log 2>&1 || {
    echo "FAIL: BusyBox build; see /tmp/bit-busybox-build.log" >&2
    exit 1
}
test -x busybox_unstripped
./busybox_unstripped echo busybox >/tmp/bit-busybox-echo
grep -qx busybox /tmp/bit-busybox-echo
printf 'busybox\n' >/tmp/bit-busybox-input
./busybox_unstripped cat /tmp/bit-busybox-input >/tmp/bit-busybox-cat
grep -qx busybox /tmp/bit-busybox-cat
echo "PASS: bit-tcc compiles and links BusyBox 1.35 (minimal config)"
