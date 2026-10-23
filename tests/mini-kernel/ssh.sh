#!/bin/sh
# Build a minimal Linux 7.2.3 image with BusyBox, Lua 5.4 and OpenSSH.
# Kernel source must be a clean copy; this script does not modify it.
set -eu

ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
KERNEL_SRC=${KERNEL_SRC:-/opt/linux-7.2.3-clean}
BUSYBOX=${BUSYBOX:-"$ROOT/build/busybox-full/busybox_unstripped"}
LUA=${LUA:-"$ROOT/build/lua/src/lua"}
SSHD=${SSHD:-/usr/sbin/sshd}
BUILD=${BUILD:-/opt/mini-kernel-ssh}
JOBS=${JOBS:-$(nproc)}
QEMU=${QEMU:-qemu-system-x86_64}
SSH_PORT=${SSH_PORT:-2222}
BOOT_TIMEOUT=${BOOT_TIMEOUT:-60}
BUILD_KERNEL=${BUILD_KERNEL:-0}

need() { [ -e "$1" ] || { echo "missing: $1" >&2; exit 1; }; }
for x in "$KERNEL_SRC/Makefile" "$BUSYBOX" "$LUA" "$SSHD"; do need "$x"; done

KERNEL_BUILD="$BUILD/kernel"
IRFS="$BUILD/initramfs"
INITRD="$BUILD/initramfs.cpio.gz"
BZIMAGE="$KERNEL_BUILD/arch/x86/boot/bzImage"
HOSTKEY="$BUILD/hostkey"
CLIENTKEY="$BUILD/client_key"
LOG="$BUILD/qemu.log"

if [ "$BUILD_KERNEL" = 1 ] || [ ! -e "$BZIMAGE" ]; then
    mkdir -p "$KERNEL_BUILD"
    make -C "$KERNEL_SRC" O="$KERNEL_BUILD" defconfig >/tmp/mini-ssh-defconfig.log 2>&1
    CONFIG="$KERNEL_BUILD/.config"
    for opt in \
        CONFIG_64BIT CONFIG_X86_64 CONFIG_BLK_DEV_INITRD CONFIG_RD_GZIP \
        CONFIG_DEVTMPFS CONFIG_DEVTMPFS_MOUNT CONFIG_DEVPTS_FS CONFIG_UNIX98_PTYS \
        CONFIG_TMPFS CONFIG_PROC_FS CONFIG_SYSFS CONFIG_TTY \
        CONFIG_SERIAL_8250 CONFIG_SERIAL_8250_CONSOLE \
        CONFIG_BINFMT_ELF CONFIG_BINFMT_SCRIPT CONFIG_PRINTK \
        CONFIG_NET CONFIG_INET CONFIG_PACKET CONFIG_UNIX CONFIG_NETDEVICES \
        CONFIG_VIRTIO CONFIG_VIRTIO_PCI CONFIG_VIRTIO_NET CONFIG_VIRTIO_BLK \
        CONFIG_E1000 CONFIG_EXT4_FS CONFIG_SQUASHFS; do
        "$KERNEL_SRC/scripts/config" --file "$CONFIG" -e "$opt"
    done
    "$KERNEL_SRC/scripts/config" --file "$CONFIG" -d CONFIG_MODULES
    "$KERNEL_SRC/scripts/config" --file "$CONFIG" -d CONFIG_DEBUG_INFO_BTF
    make -C "$KERNEL_SRC" O="$KERNEL_BUILD" olddefconfig >/tmp/mini-ssh-olddefconfig.log 2>&1
    make -C "$KERNEL_SRC" O="$KERNEL_BUILD" -j"$JOBS" bzImage >/tmp/mini-ssh-kernel.log 2>&1
fi
need "$BZIMAGE"

rm -rf "$IRFS"
mkdir -p "$IRFS"/bin "$IRFS"/sbin "$IRFS"/usr/sbin "$IRFS"/dev "$IRFS"/proc \
    "$IRFS"/sys "$IRFS"/run "$IRFS"/tmp "$IRFS"/etc/ssh "$IRFS"/root/.ssh \
    "$IRFS"/var/empty "$IRFS"/lib64 "$IRFS"/lib/x86_64-linux-gnu "$IRFS"/etc/ssh

cp "$BUSYBOX" "$IRFS/bin/busybox"
chmod +x "$IRFS/bin/busybox"
ln -s busybox "$IRFS/bin/sh"
cp "$LUA" "$IRFS/bin/lua"
cp "$SSHD" "$IRFS/usr/sbin/sshd"
chmod +x "$IRFS/usr/sbin/sshd"

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
copy_libs "$IRFS/usr/sbin/sshd"
cp -L /lib/x86_64-linux-gnu/libnss_files.so.2 "$IRFS/lib/x86_64-linux-gnu/"

rm -f "$HOSTKEY" "$HOSTKEY.pub" "$CLIENTKEY" "$CLIENTKEY.pub"
ssh-keygen -q -t ed25519 -N '' -f "$HOSTKEY"
ssh-keygen -q -t ed25519 -N '' -f "$CLIENTKEY"
cp "$HOSTKEY" "$IRFS/etc/ssh/ssh_host_ed25519_key"
cp "$HOSTKEY.pub" "$IRFS/etc/ssh/ssh_host_ed25519_key.pub"
cp "$CLIENTKEY.pub" "$IRFS/root/.ssh/authorized_keys"
chmod 600 "$IRFS/etc/ssh/ssh_host_ed25519_key" "$IRFS/root/.ssh/authorized_keys"
chmod 700 "$IRFS/root/.ssh"
chmod 755 "$IRFS/var/empty"

ROOT_HASH=$(openssl passwd -6 root)
cat > "$IRFS/etc/passwd" <<EOF
root:x:0:0:root:/root:/bin/sh
sshd:x:74:74:sshd:/var/empty:/bin/false
EOF
cat > "$IRFS/etc/group" <<EOF
root:x:0:
sshd:x:74:
EOF
cat > "$IRFS/etc/shadow" <<EOF
root:$ROOT_HASH:19000:0:99999:7:::
sshd:!:19000:0:99999:7:::
EOF
chmod 640 "$IRFS/etc/shadow"

cat > "$IRFS/etc/ssh/sshd_config" <<'EOF'
Port 22
ListenAddress 0.0.0.0
HostKey /etc/ssh/ssh_host_ed25519_key
PermitRootLogin prohibit-password
PubkeyAuthentication yes
PasswordAuthentication no
KbdInteractiveAuthentication no
UsePAM no
PidFile /run/sshd.pid
LogLevel VERBOSE
EOF

cat > "$IRFS/init" <<'EOF'
#!/bin/sh
/bin/busybox mount -t proc proc /proc
/bin/busybox mount -t sysfs sys /sys
/bin/busybox mount -t devtmpfs dev /dev 2>/dev/null || true
/bin/busybox mkdir -p /dev/pts /run/sshd /var/empty
/bin/busybox mount -t devpts devpts /dev/pts
/bin/busybox --install -s /bin
/bin/busybox ip link set eth0 up
/bin/busybox ip addr add 10.0.2.15/24 dev eth0
/bin/busybox ip route add default via 10.0.2.2
/bin/busybox echo "MINI-SSH: $(/bin/busybox uname -a)"
/bin/busybox echo "BUSYBOX: ok"
/bin/lua -e 'print("LUA: " .. _VERSION .. " ok")'
/usr/sbin/sshd -D -e &
exec /bin/sh
EOF
chmod +x "$IRFS/init"

(cd "$IRFS" && find . -print0 | cpio --null -o -H newc --owner=0:0 2>/dev/null | gzip -9) > "$INITRD"
need "$INITRD"

echo "kernel:  $BZIMAGE"
echo "initrd:  $INITRD"
echo "key:     $CLIENTKEY"
echo "ssh:     ssh -p $SSH_PORT -i $CLIENTKEY root@127.0.0.1"

"$QEMU" \
    -m 1G \
    -kernel "$BZIMAGE" \
    -initrd "$INITRD" \
    -append "console=ttyS0 rdinit=/init panic=-1" \
    -nographic -no-reboot \
    -netdev user,id=net0,hostfwd=tcp::"$SSH_PORT"-:22 \
    -device virtio-net-pci,netdev=net0 \
    >"$LOG" 2>&1 &
QPID=$!
trap 'kill "$QPID" 2>/dev/null || true; wait "$QPID" 2>/dev/null || true' EXIT INT TERM

i=0
while [ "$i" -lt "$BOOT_TIMEOUT" ]; do
    if ssh -p "$SSH_PORT" -i "$CLIENTKEY" \
        -o BatchMode=yes -o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null \
        -o ConnectTimeout=1 root@127.0.0.1 true >/dev/null 2>&1; then
        break
    fi
    i=$((i + 1))
    sleep 1
done
if [ "$i" -ge "$BOOT_TIMEOUT" ]; then
    echo "FAIL: SSH did not come up; see $LOG" >&2
    tail -40 "$LOG" >&2
    exit 1
fi

ssh -p "$SSH_PORT" -i "$CLIENTKEY" \
    -o BatchMode=yes -o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null \
    root@127.0.0.1 'echo SSH_OK; uname -a; /bin/lua -v; /bin/busybox | head -1'
echo "PASS: minimal SSH image booted in QEMU"
