# 自定义操作系统镜像

本文描述当前最小 Linux 镜像的构建方式、运行原理，以及从 QEMU 镜像走向真实机顶盒镜像的路线。

当前目标不是完整发行版，而是一个可验证的最小系统：

```text
Linux 7.2.3
BusyBox 1.36.1
Lua 5.4.7
OpenSSH sshd
QEMU x86_64
```

已验证输出：

```text
SSH_OK
Linux (none) 7.2.3
Lua 5.4.7
BusyBox v1.36.1
```

---

## 1. 目录与脚本

```text
tests/mini-kernel/
  build.sh       最小 initramfs，不含 SSH
  ssh.sh         内核 + BusyBox + Lua + OpenSSH，并自动测试 SSH
  run-ssh.sh     启动已构建的 SSH 镜像
  README.md      简版使用说明
```

相关产物：

```text
/opt/mini-kernel-7.2.3/
  kernel/arch/x86/boot/bzImage
  initramfs.cpio.gz

/opt/mini-kernel-ssh/
  kernel/arch/x86/boot/bzImage
  initramfs.cpio.gz
  client_key
  qemu.log
```

---

## 2. 使用方式

### 2.1 构建并测试 SSH 镜像

```sh
cd /home/quqiufeng/bit
sh tests/mini-kernel/ssh.sh
```

脚本会：

```text
1. 检查内核 bzImage
2. 不存在时从 /opt/linux-7.2.3-clean 构建
3. 生成 initramfs
4. 复制 BusyBox、Lua、OpenSSH 及动态库
5. 生成 SSH host key 和客户端测试 key
6. 启动 QEMU
7. 等待 sshd
8. 通过 SSH 执行 uname/lua/busybox 测试
```

强制重建内核：

```sh
BUILD_KERNEL=1 sh tests/mini-kernel/ssh.sh
```

### 2.2 只构建无 SSH 的最小镜像

```sh
sh tests/mini-kernel/build.sh
```

该脚本使用已有内核：

```text
/opt/linux/src/linux-7.2.3/arch/x86/boot/bzImage
```

需要重新构建时设置：

```sh
BUILD_KERNEL=1 KERNEL_SRC=/opt/linux-7.2.3-clean sh tests/mini-kernel/build.sh
```

### 2.3 启动已构建的 SSH 镜像

```sh
sh tests/mini-kernel/run-ssh.sh
```

另一个终端登录：

```sh
ssh -p 2222 -i /opt/mini-kernel-ssh/client_key root@127.0.0.1
```

当前 initramfs 仅启用公钥登录，私钥是：

```text
/opt/mini-kernel-ssh/client_key
```

不要发布这个私钥。

### 2.4 手动验证与验收

自动测试：

```sh
cd /home/quqiufeng/bit
sh tests/mini-kernel/ssh.sh
```

看到以下输出即通过：

```text
SSH_OK
Linux (none) 7.2.3 ...
Lua 5.4.7 ...
BusyBox v1.36.1 ...
PASS: minimal SSH image booted in QEMU
```

交互式启动：终端 A 执行：

```sh
sh tests/mini-kernel/run-ssh.sh
```

终端 B 登录：

```sh
ssh -p 2222 \
  -i /opt/mini-kernel-ssh/client_key \
  -o StrictHostKeyChecking=no \
  -o UserKnownHostsFile=/dev/null \
  root@127.0.0.1
```

Guest 内检查：

```sh
uname -a
cat /proc/version
cat /proc/cmdline
lua -v
busybox | head
ip addr
mount
ls -l /dev
ps
```

不登录，直接远程验收：

```sh
ssh -p 2222 \
  -i /opt/mini-kernel-ssh/client_key \
  -o BatchMode=yes \
  -o StrictHostKeyChecking=no \
  -o UserKnownHostsFile=/dev/null \
  root@127.0.0.1 \
  'echo SSH_OK; uname -a; lua -v; busybox | head -1'
```

检查镜像文件：

```sh
ls -lh /opt/mini-kernel-ssh/kernel/arch/x86/boot/bzImage
ls -lh /opt/mini-kernel-ssh/initramfs.cpio.gz
file /opt/mini-kernel-ssh/kernel/arch/x86/boot/bzImage
gzip -dc /opt/mini-kernel-ssh/initramfs.cpio.gz | cpio -it | head
```

查看启动日志：

```sh
tail -80 /opt/mini-kernel-ssh/qemu.log
```

重点确认：

```text
Run /init as init process
MINI-SSH: Linux ... 7.2.3
BUSYBOX: ok
LUA: Lua 5.4 ok
```

验证 initramfs 无持久化：

```sh
echo test >/tmp/test
reboot -f
```

重新启动后：

```sh
ls /tmp/test
```

应不存在，因为当前 rootfs 在内存中。

验收清单：

```text
[ ] Linux 7.2.3 启动
[ ] /init 运行
[ ] BusyBox 可用
[ ] Lua 5.4.7 可运行
[ ] eth0 有 10.0.2.15
[ ] SSH 可用
[ ] /proc /sys /dev /dev/pts 挂载正常
[ ] 重启后修改丢失（initramfs）
```

注意：这里验证的是 QEMU 镜像功能，不是完整编译器血统。内核仍由宿主 GCC 13.3 构建，sshd 仍是宿主 OpenSSH 二进制。

---

## 3. 运行原理

### 3.1 两段式启动

```text
QEMU
  -> bzImage
  -> initramfs.cpio.gz
  -> /init
  -> BusyBox + Lua + sshd
```

initramfs 是内存文件系统，不依赖磁盘：

```text
内核加载 initrd
  -> 解压 cpio
  -> 挂到 rootfs
  -> 执行 /init
```

因此当前镜像没有持久化，重启后所有修改丢失。

### 3.2 init 的职责

`/init` 是用户态第一号进程，当前执行：

```text
mount proc
mount sysfs
mount devtmpfs
mount devpts
创建 /run/sshd
配置 eth0
启动 sshd
启动 /bin/sh
```

网络使用 QEMU user networking 的固定地址：

```text
guest: 10.0.2.15/24
gateway: 10.0.2.2
hostfwd: host tcp/2222 -> guest tcp/22
```

### 3.3 动态链接

BusyBox、Lua 和 OpenSSH 都是动态链接 ELF，因此 initramfs 里需要：

```text
/lib64/ld-linux-x86-64.so.2
/lib/x86_64-linux-gnu/libc.so.6
/lib/x86_64-linux-gnu/libm.so.6
libnss_files.so.2
OpenSSH 依赖的 libcrypto/libpam/libwrap 等
```

脚本通过 `ldd` 自动收集这些库。

### 3.4 SSH 权限

sshd 需要：

```text
/etc/passwd
/etc/group
/etc/shadow
/etc/ssh/ssh_host_ed25519_key
/root/.ssh/authorized_keys
/run/sshd
/var/empty
/dev/pts
```

cpio 必须以 root 身份记录文件：

```text
cpio --owner=0:0
```

否则 sshd 会报：

```text
bad ownership or modes for file /root/.ssh/authorized_keys
```

### 3.5 QEMU 驱动与宿主驱动

QEMU 不需要把宿主物理硬件驱动给 guest。guest 只需要 QEMU 模拟设备的驱动：

```text
virtio-net
virtio-blk
virtio-pci
serial
devtmpfs
ext4/squashfs
```

宿主驱动由 QEMU 进程使用，例如宿主网卡、磁盘、GPU。guest 看不到 RTX 3080、WiFi 或机顶盒 SoC 的真实驱动。

只有 VFIO 直通时，guest 才需要对应真实设备驱动。

---

## 4. 当前边界

```text
x86_64 only
QEMU virtio 设备
initramfs，无持久化
内核由宿主 GCC 13.3 构建
OpenSSH 使用宿主二进制
没有 rootfs、ISO、U-Boot、安装器
没有真实机顶盒 SoC 驱动
```

这不是完整 LFS，也不是可安装发行版，而是一个用于验证内核、用户态和网络/SSH 的最小实验镜像。

---

## 5. 下一步：可启动 ISO

从 initramfs 到 ISO 需要增加：

```text
1. rootfs
   ext4 或 squashfs
   BusyBox/Lua/OpenSSH/init 配置

2. initrd
   只负责挂载 rootfs，或保留当前 initramfs

3. bootloader
   GRUB BIOS + UEFI
   或 syslinux/isolinux

4. ISO
   xorriso -as mkisofs
   El Torito + EFI 分区

5. 安装器
   分区、mkfs、复制 rootfs、grub-install
```

建议先做 **live ISO**：

```text
ISO 启动
  -> 内核 + initramfs
  -> 挂载 squashfs
  -> 进入 BusyBox shell
  -> 网络/SSH 可用
```

再做 **installer ISO**：

```text
live 环境
  -> 选择磁盘
  -> 分区
  -> 格式化
  -> 复制 rootfs
  -> 安装 bootloader
  -> 重启进入硬盘系统
```

本机已有：

```text
xorriso
grub-mkrescue
grub-install
```

`mtools` 未安装，UEFI ISO 制作前可能需要补齐。

---

## 6. 下一步：真实机顶盒镜像

机顶盒不是 QEMU，必须面对真实 SoC 和启动链。

### 6.1 先收集硬件信息

```text
型号：
SoC：
架构：arm64 / arm / mips
内存：
存储：eMMC / NAND / SD
启动方式：U-Boot / 原厂 recovery / 串口
串口引脚：
网口 / WiFi：
```

### 6.2 保留原厂 bootloader

第一原则：不要先动 eMMC/NAND。

```text
用 SD 卡或 USB 启动
保留原厂 U-Boot 和 recovery
确认串口能看日志
确认可以恢复后再写内部存储
```

### 6.3 需要的构件

```text
1. 对应 SoC 的内核源码/补丁
2. 设备树 DTB
3. U-Boot 启动脚本：boot.scr / extlinux.conf / FIT image
4. 存储驱动：SDHCI / eMMC / NAND / NVMe
5. 网络驱动：以太网 / WiFi
6. 显示/输入：HDMI / framebuffer / IR
7. rootfs：BusyBox + glibc/musl + SSH + Lua
8. 启动介质：SD / USB / eMMC 镜像
```

### 6.4 启动链

典型 ARM 机顶盒：

```text
BootROM
  -> U-Boot
  -> kernel + dtb
  -> initramfs 或 rootfs
  -> /init
```

需要确认：

```text
U-Boot 是否允许从 SD/USB 启动
是否锁定签名/secure boot
内核是否需要 SoC 厂商补丁
DTB 是否与板卡匹配
```

### 6.5 测试顺序

```text
1. 串口 + U-Boot，能加载 kernel/dtb
2. initramfs 启动到 BusyBox
3. 网络驱动可用
4. SSH 可登录
5. 挂载 SD/eMMC rootfs
6. 持久化启动
7. 再考虑安装器/写 eMMC
```

### 6.6 x86 小机箱验证

本机另有一台 x86 小机箱，可作为“裸机 x86_64”验证平台：

```text
QEMU x86_64      -> 快速迭代
x86 小机箱       -> 裸机启动、真实固件、真实磁盘、真实网卡
ARM/MIPS 机顶盒  -> 交叉编译、U-Boot/DTB、SoC 驱动
```

x86 小机箱上优先验证：

```text
BIOS/UEFI 启动
GRUB 菜单
内核 + initramfs / rootfs
网卡 DHCP 或静态 IP
SSH 登录
磁盘读写与持久化
重启 / 断电重启
从上一版镜像回滚
```

建议先做 live USB/ISO，不直接覆盖内部磁盘：

```text
制作 live ISO
  -> USB 启动
  -> 走完网络/SSH/存储测试
  -> 再决定是否安装到内部磁盘
```

x86 小机箱的验证价值：

```text
证明镜像不只能在 QEMU 的 virtio 环境运行
证明真实固件、真实磁盘、真实网卡可用
证明 rootfs/启动链/回滚流程可落地
```

机顶盒的验证价值：

```text
证明交叉编译、目标 ABI、U-Boot/DTB 和 SoC 驱动可用
```

两者合起来，才构成“x86 裸机 + 交叉目标硬件”的完整工程证据。

### 6.7 努比亚 Z18 / ARM64 验证

实测设备：

```text
型号：Nubia Z18 / NX606J
SoC：Qualcomm SDM845（骁龙 845）
架构：ARM64
内存/存储：6/8GB RAM，64/128GB UFS 2.1
当前系统：LineageOS 22.2（Android 15），已成功启动
Recovery：Lineage Recovery 22.2
Bootloader：Device unlocked: true；Nubia fastboot unlocked: true
```

已完成的关键工作：

```text
EDL 备份关键分区
EDL 写入 V2.31 modem/xbl/tz/hyp 等固件
写入自定义 ABL + Lineage Recovery
设置 Nubia fastboot unlock flag
Factory data reset
sideload LineageOS 22.2
首次启动成功
```

详细刷机记录：

```text
https://github.com/quqiufeng/my-agent/blob/main/nubia.md
```

现状边界：

```text
Z18 可以刷 LineageOS，已经验证
没有成熟的桌面 Linux / Ubuntu Touch / postmarketOS 日常方案
通用最小 Linux 仍属于实验，不是当前稳定路径
```

源码构建 ROM 也要分层：

```text
1. 刷官方/预编译 ROM
   只是安装，不是源码构建

2. 基于 AOSP/LineageOS 源码构建 ROM
   是真正的源码编译
   但仍使用 AOSP prebuilt Clang、Soong/Ninja、vendor blobs

3. 用本项目工具链构建整个 ROM
   Android framework/HAL/Bionic 仍不现实
```

对本项目最有价值的混合验证是：

```text
LineageOS 源码构建 ROM
  + 自有 GCC 14.2 交叉构建 kernel
  + 自有工具链构建 BusyBox/Lua/SSH（独立环境）
  + AOSP framework 仍用 prebuilt Clang
```

这可以验证：

```text
自有交叉编译器能生成该设备可启动的内核
目标 ABI / 启动链 / 分区 / 驱动集成正确
自有工具链能生成真实 ARM64 用户态
源码构建和镜像打包流程可重复
```

但不能写成：

```text
整个 Android ROM 都由本项目编译器构建
```

因此手机在项目里应定位为“高级 ARM64 验证平台”，不是第一步：

```text
A. 自建 Android ROM
   AOSP/LineageOS + device tree + vendor blobs + prebuilt clang
   能验证系统集成，但不等于自有编译器血统

B. 最小 Linux initramfs（实验）
   需要适配 SDM845 内核/DTB、USB gadget、显示/存储驱动
   先用 fastboot boot，不直接写 boot/recovery

C. 完全自举手机 OS
   长期目标，不作为当前阶段目标
```

如果做 B，安全顺序是：

```text
1. 保留 EDL loader 和关键分区备份
2. 确认 fastboot oem device-info 的两个 unlocked 都是 true
3. 基于已有内核/DTB 先做 fastboot boot 临时镜像
4. 启动到 initramfs 后再做 USB 网络/SSH
5. 不先写 boot/recovery
6. 每次改动前确认可以 EDL 恢复
```

手机测试验收项：

```text
[ ] fastboot boot 能加载自定义 kernel/initramfs
[ ] 内核启动到 BusyBox
[ ] Lua 5.4 可运行
[ ] USB 网络或 WiFi 可用
[ ] SSH 可登录（如果做得到）
[ ] 重启能回到 LineageOS 或自定义系统
[ ] EDL 备份可恢复
```

手机行的验收状态必须区分“LineageOS 已验证”和“本项目自定义镜像已验证”，不能混为一谈。

---

## 7. 编译器血统

当前内核和 SSH 镜像仍使用：

```text
宿主 GCC 13.3 构建内核
宿主 OpenSSH 二进制
系统 glibc/crt
```

后续可以逐步替换：

```text
GCC 14.2 -> 构建内核
自有 binutils -> 构建内核/用户态
自有 glibc/sysroot -> 构建 BusyBox/Lua/SSH
自有 OpenSSH/Dropbear -> 替代宿主 sshd
```

但这应建立在“镜像能启动、网络可用、可恢复”之后，避免同时引入硬件和工具链两类问题。

---

## 8. 推荐执行顺序

```text
P1 稳定当前 QEMU SSH 镜像
P2 做 x86 live ISO（BIOS + UEFI）
P3 在 x86 小机箱完成 USB 启动、网络、SSH、磁盘、回滚验证
P4 做可写 ext4 磁盘镜像和 installer ISO
P5 选定一款机顶盒，确认 SoC/串口/U-Boot
P6 构建该 SoC 的内核 + DTB + 驱动
P7 做 SD/USB 启动镜像并在机顶盒实测
P8 手机 ARM64（高级）：基于 Z18/LineageOS 内核评估自定义 initramfs，先 fastboot boot
P9 手机 USB 网络/SSH/重启/回滚验证（如果继续推进）
P10 再考虑写入 eMMC/NAND 或手机 boot 分区
P11 把 QEMU、x86 裸机、机顶盒、手机四类验证固化为同一套回归矩阵
```

最终目标是：

```text
同一套用户态和启动脚本，
先在 QEMU 上验证，
再在 x86 小机箱上做裸机验证，
再针对真实 SoC 做实机验证，
最后把内核、DTB、驱动和启动链按平台替换。
```

验收矩阵：

| 平台 | 启动 | 网络 | SSH | 持久化 | 回滚 |
| --- | --- | --- | --- | --- | --- |
| QEMU x86_64 | 已验证 | 已验证 | 已验证 | initramfs，无 | 重建镜像 |
| x86 小机箱 | 待验证 | 待验证 | 待验证 | 待验证 | 待验证 |
| ARM/MIPS 机顶盒 | 待验证 | 待验证 | 待验证 | 待验证 | 待验证 |
| 努比亚 Z18（SDM845/ARM64） | LineageOS 已验证；自定义镜像待验证 | 自定义镜像待验证 | 自定义镜像待验证 | LineageOS 已验证；自定义待验证 | EDL 备份可用 |
