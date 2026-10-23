#!/bin/sh
# Build a minimal x86_64 Linux + BusyBox + Lua initramfs and boot it in QEMU.
# Kernel source is external; this script does not modify it.
set -eu

ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
KERNEL_SRC=${KERNEL_SRC:-/opt/linux/src/linux-7.2.3}
BUSYBOX=${BUSYBOX:-"$ROOT/build/busybox-full/busybox_unstripped"}
LUA=${LUA:-"$ROOT/build/lua/src/lua"}
BUILD=${BUILD:-/opt/mini-kernel-7.2.3}
JOBS=${JOBS:-$(nproc)}
QEMU=${QEMU:-qemu-system-x86_64}

need() { [ -e "$1" ] || { echo "missing: $1" >&2; exit 1; }; }
need "$KERNEL_SRC/Makefile"
need "$BUSYBOX"
need "$LUA"

KERNEL_BUILD="$BUILD/kernel"
IRFS="$BUILD/initramfs"
INITRD="$BUILD/initramfs.cpio.gz"
BZIMAGE=${BZIMAGE:-"$KERNEL_SRC/arch/x86/boot/bzImage"}

mkdir -p "$IRFS"
if [ "${BUILD_KERNEL:-0}" = 1 ] || [ ! -e "$BZIMAGE" ]; then
    mkdir -p "$KERNEL_BUILD"
    make -C "$KERNEL_SRC" O="$KERNEL_BUILD" defconfig >/tmp/mini-kernel-defconfig.log 2>&1
    CONFIG="$KERNEL_BUILD/.config"
    for opt in \
        CONFIG_64BIT CONFIG_X86_64 CONFIG_BLK_DEV_INITRD CONFIG_RD_GZIP \
        CONFIG_DEVTMPFS CONFIG_DEVTMPFS_MOUNT CONFIG_TMPFS CONFIG_PROC_FS \
        CONFIG_SYSFS CONFIG_TTY CONFIG_SERIAL_8250 CONFIG_SERIAL_8250_CONSOLE \
        CONFIG_BINFMT_ELF CONFIG_BINFMT_SCRIPT CONFIG_PRINTK; do
        "$KERNEL_SRC/scripts/config" --file "$CONFIG" -e "$opt"
    done
    "$KERNEL_SRC/scripts/config" --file "$CONFIG" -d CONFIG_MODULES
    "$KERNEL_SRC/scripts/config" --file "$CONFIG" -d CONFIG_DEBUG_INFO_BTF
    make -C "$KERNEL_SRC" O="$KERNEL_BUILD" olddefconfig >/tmp/mini-kernel-olddefconfig.log 2>&1
    make -C "$KERNEL_SRC" O="$KERNEL_BUILD" -j"$JOBS" bzImage >/tmp/mini-kernel-build.log 2>&1
    BZIMAGE="$KERNEL_BUILD/arch/x86/boot/bzImage"
fi
need "$BZIMAGE"

rm -rf "$IRFS"
mkdir -p "$IRFS"/bin "$IRFS"/dev "$IRFS"/proc "$IRFS"/sys "$IRFS"/tmp "$IRFS"/etc "$IRFS"/lib64
cp "$BUSYBOX" "$IRFS/bin/busybox"
chmod +x "$IRFS/bin/busybox"
ln -s busybox "$IRFS/bin/sh"
cp "$LUA" "$IRFS/bin/lua"

copy_libs() {
    ldd "$1" 2>/dev/null | awk '$2 == "=>" {print $3} $1 ~ /^\// {print $1}' | while read -r lib; do
        [ -e "$lib" ] || continue
        dest="$IRFS$lib"
        mkdir -p "$(dirname "$dest")"
        cp -L "$lib" "$dest"
    done
}
cp -L /lib64/ld-linux-x86-64.so.2 "$IRFS/lib64/ld-linux-x86-64.so.2"
copy_libs "$IRFS/bin/busybox"
copy_libs "$IRFS/bin/lua"

cat > "$IRFS/init" <<'EOF'
#!/bin/sh
/bin/busybox mount -t proc proc /proc
/bin/busybox mount -t sysfs sys /sys
/bin/busybox mount -t devtmpfs dev /dev 2>/dev/null || true
/bin/busybox echo "MINI-KERNEL: $(/bin/busybox uname -a)"
/bin/busybox echo "BUSYBOX: ok"
/bin/lua -e 'print("LUA: " .. _VERSION .. " ok")'
/bin/busybox sync
exec /bin/busybox reboot -f
EOF
chmod +x "$IRFS/init"

(cd "$IRFS" && find . -print0 | cpio --null -o -H newc 2>/dev/null | gzip -9) > "$INITRD"
need "$INITRD"

echo "kernel:  $BZIMAGE"
echo "initrd:  $INITRD"
echo "qemu:    $QEMU"

timeout "${BOOT_TIMEOUT:-60}" "$QEMU" \
    -m "${QEMU_MEM:-256M}" \
    -kernel "$BZIMAGE" \
    -initrd "$INITRD" \
    -append "console=ttyS0 rdinit=/init panic=-1" \
    -nographic -no-reboot
