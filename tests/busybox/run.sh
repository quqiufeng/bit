#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
BUSYBOX_SRC=${BUSYBOX_SRC:-/tmp/busybox-1.36.1}
CC=${CC:-"$ROOT/build/tcc/bit-tcc"}
OUT=${OUT:-"$ROOT/build/busybox-full"}
PATCH="$ROOT/tests/busybox/busybox-1.36.1-tcc.patch"
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
if patch -p1 --dry-run < "$PATCH" >/dev/null 2>&1; then
    patch -p1 < "$PATCH"
elif ! patch -p1 -R --dry-run < "$PATCH" >/dev/null 2>&1; then
    echo "FAIL: BusyBox patch does not apply to $BUSYBOX_SRC" >&2
    exit 1
fi
make defconfig >/dev/null
for applet in FEATURE_CLEAN_UP FEATURE_SH_NOFORK FEATURE_SUN_LABEL; do
    sed -i "s/^# CONFIG_${applet} is not set/CONFIG_${applet}=y/" .config
done
sed -i 's/^CONFIG_EXTRA_LDLIBS=.*/CONFIG_EXTRA_LDLIBS="resolv"/' .config
yes '' | make oldconfig >/dev/null
cat > "$OUT/bit-tcc-cc" <<EOF
#!/bin/sh
for arg in "\$@"; do
    case "\$arg" in
        *.S|*.s) exec gcc "\$@" ;;
    esac
done
exec "$CC" "\$@"
EOF
chmod +x "$OUT/bit-tcc-cc"
make clean >/dev/null 2>&1 || true
make -j2 CC="$OUT/bit-tcc-cc" HOSTCC=gcc SKIP_STRIP=y >/tmp/bit-busybox-full-build.log 2>&1 || {
    echo "FAIL: BusyBox full build; see /tmp/bit-busybox-full-build.log" >&2
    exit 1
}
test -x busybox_unstripped
./busybox_unstripped echo full-busybox >/tmp/bit-busybox-full-echo
grep -qx full-busybox /tmp/bit-busybox-full-echo
./busybox_unstripped sh -c 'echo from-sh' >/tmp/bit-busybox-full-sh
grep -qx from-sh /tmp/bit-busybox-full-sh
./busybox_unstripped --list | grep -qx ash
echo "PASS: bit-tcc compiles and runs BusyBox 1.36.1 defconfig"
