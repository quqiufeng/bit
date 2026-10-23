#!/bin/sh
# Boot the prebuilt minimal SSH image.  Keep the QEMU console on stdio and
# forward host tcp/2222 to guest sshd.
set -eu

BUILD=${BUILD:-/opt/mini-kernel-ssh}
SSH_PORT=${SSH_PORT:-2222}
QEMU=${QEMU:-qemu-system-x86_64}
KERNEL="$BUILD/kernel/arch/x86/boot/bzImage"
INITRD="$BUILD/initramfs.cpio.gz"

[ -e "$KERNEL" ] || { echo "missing $KERNEL; run ssh.sh first" >&2; exit 1; }
[ -e "$INITRD" ] || { echo "missing $INITRD; run ssh.sh first" >&2; exit 1; }

echo "ssh -p $SSH_PORT -i $BUILD/client_key root@127.0.0.1"
exec "$QEMU" \
    -m 1G \
    -kernel "$KERNEL" \
    -initrd "$INITRD" \
    -append "console=ttyS0 rdinit=/init panic=-1" \
    -nographic -no-reboot \
    -netdev user,id=net0,hostfwd=tcp::"$SSH_PORT"-:22 \
    -device virtio-net-pci,netdev=net0
