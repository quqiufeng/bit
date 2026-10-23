# BusyBox 1.36.1 / TCC shell 试用

本目录维护 BusyBox 1.36.1 的 TCC 兼容补丁和完整构建脚本。

## 构建

源码默认在：

```text
/opt/busybox-1.36.1
```

构建：

```sh
cd /home/quqiufeng/bit
./tests/busybox/run.sh
```

产物：

```text
build/busybox-full/busybox_unstripped
```

构建边界：

```text
C 代码：bit-tcc 编译
最终 ELF：bit-tcc 链接
.S 汇编：系统 as/gcc
glibc / crt / 动态加载器：系统提供
```

## 直接运行 applet

```sh
cd /home/quqiufeng/bit/build/busybox-full

./busybox_unstripped --list
./busybox_unstripped echo hello
./busybox_unstripped uname -a
./busybox_unstripped ls -l
```

## 进入交互式 shell

```sh
cd /home/quqiufeng/bit/build/busybox-full
./busybox_unstripped sh
```

也可以使用：

```sh
./busybox_unstripped ash
```

强制交互模式：

```sh
./busybox_unstripped sh -i
```

进入后可直接执行 BusyBox 内置 applet 和 shell builtin：

```sh
help
busybox
echo hello
uname -a
pwd
ls
mkdir /tmp/bb-test
cd /tmp/bb-test
touch a b c
ls
cat /etc/hostname
exit
```

`help` 显示的是 shell builtin，例如：

```text
. : [ [[ alias bg break cd chdir command continue echo eval exec
exit export false fg getopts hash help history jobs kill let
local printf pwd read readonly return set shift sleep source
test times trap true type ulimit umask unalias unset wait
```

## 创建临时 applet 命令

不想每次输入 `./busybox_unstripped <applet>`，可以创建临时 symlink：

```sh
cd /home/quqiufeng/bit/build/busybox-full
mkdir -p /tmp/bb-bin
./busybox_unstripped --install -s /tmp/bb-bin
export PATH=/tmp/bb-bin:$PATH

sh
ls
cat /etc/hostname
```

注意：

```text
只把 /tmp/bb-bin 加到 PATH 前面
不要覆盖 /bin/sh、/bin/ls、/bin/cat
```

退出当前 shell：

```sh
exit
```

## 当前能证明什么

已经证明：

```text
TCC 能编译完整 BusyBox 1.36.1 defconfig
TCC 能链接 BusyBox
BusyBox ash 可以交互运行
BusyBox applet 可以执行
```

还没有证明：

```text
BusyBox 可以独立启动系统
BusyBox 可以作为 PID 1
BusyBox 已经静态链接
BusyBox 可以脱离宿主 glibc
```

当前仍然是：

```text
BusyBox 作为宿主 Linux 上的普通用户态进程运行
kernel   = 宿主内核
glibc    = 宿主 glibc
rootfs   = 宿主文件系统
init     = 不是 BusyBox
```

## 相关文件

```text
tests/busybox/run.sh
tests/busybox/busybox-1.36.1-tcc.patch
/opt/busybox-1.36.1
build/busybox-full/busybox_unstripped
```
