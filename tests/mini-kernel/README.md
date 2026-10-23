# Minimal Linux + BusyBox + Lua + SSH

This directory builds a small x86_64 QEMU image from an external Linux
7.2.3 source tree.  The image is an initramfs, not an installable ISO.

## Contents

```text
build.sh    kernel + BusyBox + Lua initramfs, no SSH
ssh.sh      kernel + BusyBox + Lua + OpenSSH initramfs, boots and tests SSH
run-ssh.sh  boot the prebuilt SSH image interactively
```

## Build and test SSH image

```sh
cd /home/quqiufeng/bit
sh tests/mini-kernel/ssh.sh
```

The first run builds the kernel under `/opt/mini-kernel-ssh/kernel`; later
runs reuse the existing `bzImage`.  Force a rebuild with:

```sh
BUILD_KERNEL=1 sh tests/mini-kernel/ssh.sh
```

Inputs:

```text
kernel source: /opt/linux-7.2.3-clean
BusyBox:       build/busybox-full/busybox_unstripped
Lua:           build/lua/src/lua
sshd:          /usr/sbin/sshd (host OpenSSH)
```

The script boots QEMU, waits for sshd, then checks:

```text
SSH_OK
uname -a
lua -v
busybox version
```

## Run

```sh
sh tests/mini-kernel/run-ssh.sh
```

In another terminal:

```sh
ssh -p 2222 -i /opt/mini-kernel-ssh/client_key root@127.0.0.1
```

The initramfs uses public-key root login only.  The private key is
`/opt/mini-kernel-ssh/client_key`; do not publish it.

## Boundaries

```text
x86_64 only
QEMU virtio-net only
root filesystem is initramfs (no persistence)
OpenSSH is the host binary, not built by C1/TCC/GCC
kernel is currently built by host GCC 13.3
no ISO, installer, U-Boot, or hardware-specific drivers
```

## Verification

Run the automatic SSH test:

```sh
sh tests/mini-kernel/ssh.sh
```

Interactive boot and login:

```sh
sh tests/mini-kernel/run-ssh.sh
ssh -p 2222 -i /opt/mini-kernel-ssh/client_key root@127.0.0.1
```

Full usage, boot logs, persistence checks, ISO and set-top-box plans are in
`docs/MINI_OS_IMAGE.zh-CN.md`.

The next layer is a writable rootfs + bootloader + ISO, then a board-specific
kernel/DTB for the set-top box.
