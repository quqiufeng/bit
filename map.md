# bit / TCC / GCC 自举路线图

## 0. 文档目的

本文记录从当前已经完成的

```text
C1 -> TCC 0.9.27 -> TCC 自举
```

继续推进到

```text
C1 -> TCC -> GCC 4.7.4 -> GCC 4.8+ -> 现代 GCC
```

的完整技术路线、固定源码版本、组件顺序、兼容矩阵、验收命令和风险。

本文不是承诺“TCC 能直接编译任意 GCC”。GCC 各版本对 host 编译器、C++、binutils、glibc 和构建系统的要求不同，必须按版本阶梯逐级推进。

### 0.1 战略判断：GCC 是自由软件世界的基础设施

GCC 不只是一个 C 编译器，它是自由软件世界长期形成的工具链基座：

- Linux 内核、glibc、binutils、GNU 用户态和大量发行版软件都以 GCC 为重要构建目标；
- C 和 C++ 的 ABI、libstdc++、异常、RTTI、内联汇编和 GNU 扩展，长期由 GCC 定义和验证；
- 发行版的 bootstrap、交叉编译、sysroot 和二进制兼容，围绕 GCC 形成了完整方法论；
- 现代 GCC 又反过来要求成熟的 C++ host，形成“GCC 编译 GCC”的接力链。

TCC 的价值不是替代 GCC，而是用极小的代码量完成最底层自举，成为进入 GCC 世界的钥匙：

```text
C1       最底层可信自举
TCC      轻量 C 编译器和 GCC 入口
GCC      自由软件工具链和 ABI 生态的基石
```

因此本项目的战略目标不是“永远停留在 TCC”，而是：

```text
C1 -> TCC -> GCC 4.7.4 -> GCC 4.8+ -> 现代 GCC
```

只有走到 GCC 自举和后续版本接力，才算真正接入 Linux/自由软件生态的构建基础设施。

一句话：

```text
TCC 是钥匙，GCC 是门后的世界。
```

---

## 1. 当前基线

### 1.1 已经完成

```text
C1 编译 TCC 0.9.27
bit-tcc 运行
bit-tcc 编译 hello.c
bit-tcc 原生链接 hello
bit-tcc 构建 libtcc1.a
bit-tcc -shared 生成 .so
bit-tcc 编译并链接 BusyBox 1.35（最小配置）
bit-tcc 编译 tcc.c
tcc-stage2 运行
tcc-self1.o == tcc-self2.o
```

验证入口：

```sh
./tests/tcc/upstream.sh
```

当前结果：

```text
PASS: C1 -> TCC -> TCC self-host
```

### 1.2 当前源码位置

```text
tcc-0.9.27/                 vendored TCC 源码
tcc-0.9.27/config.h         最小固定配置
tests/tcc/upstream.sh       TCC 编译/自举测试
tools/c1-init.c             C1 对象 hosted 链接初始化
build/tcc/bit-tcc           C1 构建出的 TCC
build/tcc/tcc-stage2        TCC 自举出的 TCC
```

### 1.3 当前边界

已完成：

```text
TCC 自身编译和自举
TCC 编译普通 C 程序
TCC 构建 libtcc1.a
TCC 原生链接可执行文件
TCC -shared / -L / -l 基础共享库链接
GCC 4.7.4 兼容性侦察尚未开始
```

未完成：

```text
TCC 编译 GCC 4.7.4
GCC 4.7.4 自举
GCC 4.8+ 构建
binutils / glibc 原生构建
BusyBox / Lua / musl 实测
```

### 1.4 TCC 原生链接闭环

当前 `tests/tcc/upstream.sh` 已经验证：

```text
bit-tcc -c libtcc1.c / va_list.c / alloca86_64.S
ar -> runtime/libtcc1.a
bit-tcc -o hello examples/hello.c
bit-tcc -shared -o libtccadd.so tcc-lib.c
bit-tcc -o tcc-app tcc-app.c -L. -ltccadd
```

依赖边界：

```text
TCC 自己编译 libtcc1
TCC 自己完成 ELF 链接
bit-tcc 包装器自动加上 -B runtime
系统提供 crt1.o/crti.o/crtn.o
系统提供 glibc 和动态加载器
```

仍未覆盖：

```text
系统没有安装 TCC 头文件时的 fallback
复杂 libc 符号/版本脚本
完整 -rpath / RUNPATH
C++ 运行库
```

#### BusyBox 1.35 实测

最小 BusyBox 配置已经由 TCC 编译并链接：

```sh
BUSYBOX_SRC=/tmp/busybox-1.35.0 tests/tcc/busybox.sh
```

启用 applet：

```text
CAT / ECHO / LS / TRUE / FALSE
```

运行验证：

```text
busybox_unstripped echo hello
busybox_unstripped cat file
busybox_unstripped ls file
```

BusyBox 构建脚本未打补丁；通过的是 TCC fork 的兼容修正：

```text
__GNUC__ / glibc attribute 保留
packed union 布局
__INT_MAX__ 等预定义宏
__builtin_alloca
__USER_LABEL_PREFIX__
透明 union 参数
-Wp,-MD 依赖文件
未知链接选项忽略
归档重复扫描（--start-group 等价）
make 依赖文件格式
```

完整 BusyBox 1.36.1 `defconfig` 也已通过，使用仓库内补丁：

```sh
BUSYBOX_SRC=/opt/busybox-1.36.1 tests/busybox/run.sh
```

补丁：

```text
tests/busybox/busybox-1.36.1-tcc.patch
  - 现代 kernel headers 缺少 CBQ 定义时跳过 CBQ 打印
  - TCC 不链接 crtbegin.o，提供 __dso_handle
```

完整构建的配置补充：

```text
CONFIG_FEATURE_CLEAN_UP=y
CONFIG_FEATURE_SH_NOFORK=y
CONFIG_FEATURE_SUN_LABEL=y
CONFIG_EXTRA_LDLIBS="resolv"
```

C 代码仍由 TCC 编译和链接；仅 `.S` 汇编源交给系统 as/gcc。
这是 BusyBox 1.36.1 与 TCC 的维护补丁，不是 TCC 编译器本体的一部分。

#### 小型 C 项目验证

当前已实测并通过的项目：

| 项目 | 版本 | 源码 | 补丁 | 结果 |
| --- | --- | --- | --- | --- |
| cJSON | 1.7.18 | `/opt/cJSON-1.7.18` | 无 | PASS |
| SQLite | 3.45.3 | `/opt/sqlite-amalgamation-3450300` | 无 | PASS |
| Lua | 5.4.7 | `/opt/lua-5.4.7` | 无 | PASS |
| BusyBox | 1.36.1 | `/opt/busybox-1.36.1` | `tests/busybox/busybox-1.36.1-tcc.patch` | PASS |

验证命令：

```sh
./tests/cjson/run.sh
./tests/sqlite/run.sh
./tests/lua/run.sh
./tests/busybox/run.sh
```

覆盖点：

```text
cJSON    单文件库、malloc/stdio/math
SQLite   超大单文件 amalgamation、pthread/dl/math
Lua      多文件构建、liblua.a、hosted executable
BusyBox  完整 defconfig、shell/init 用户态目标
```

说明文档：

```text
tests/projects/README.md
```

#### 试用 BusyBox shell

构建产物：

```text
build/busybox-full/busybox_unstripped
```

进入交互式 shell：

```sh
cd /home/quqiufeng/bit/build/busybox-full
./busybox_unstripped sh
```

或：

```sh
./busybox_unstripped ash
```

进入后可以执行：

```sh
help
busybox
echo hello
uname -a
pwd
ls
cat /etc/hostname
exit
```

创建临时 applet 命令：

```sh
mkdir -p /tmp/bb-bin
./busybox_unstripped --install -s /tmp/bb-bin
export PATH=/tmp/bb-bin:$PATH
sh
```

完整试用说明见：

```text
tests/busybox/README.md
```

---

### 1.5 产物控制原则

路线约束：

```text
编译和最终 ELF 链接由我们自己的工具链完成
系统只提供输入依赖：头文件、crt、glibc、动态加载器、binutils/make
```

可以交给系统的部分：

```text
系统 glibc headers
crt1.o / crti.o / crtn.o
libc.so / libm.so / libdl.so
动态加载器
ar / make / sed / awk
```

必须由我们控制的部分：

```text
C 预处理和语义
目标代码生成
重定位
最终 ELF 链接
可执行文件 / 共享库产物
```

bootstrap 例外：

```text
bit-tcc 自身由 gcc 链接（因为是 C1 ELF 对象 + c1-init）
c1-init.o 由系统 gcc 编译
```

但 TCC 产出的最终测试产物不能退回 gcc 链接：

```text
hello        -> bit-tcc 链接
libtccadd.so -> bit-tcc 链接
tcc-app      -> bit-tcc 链接
tcc-stage2   -> bit-tcc 链接
```

当前 `tests/tcc/upstream.sh` 已经按这个原则执行：gcc 只用于 bootstrap
`bit-tcc` 和 `c1-init.o`，不再参与 `tcc-stage2` 的最终链接。

#### “干净产物”的定义

干净不是“所有代码都由我们编写”，而是：

```text
机器码由 C1/TCC 生成
目标文件重定位由 C1/TCC 完成
最终 ELF 由 C1/TCC 链接
外部输入版本固定
没有隐藏的 gcc/ld 代工最终产物
```

允许存在的外部输入：

```text
crt*.o
libc.so / libm.so / libdl.so
动态加载器
系统头文件
```

不允许：

```text
bit-tcc -o out ... 内部实际调用 gcc 链接
最终产物依赖未记录的宿主临时文件
使用未固定版本的 libc/headers 生成发布产物
```

产物验收：

```sh
readelf -h build/tcc/hello
readelf -d build/tcc/hello
readelf -d build/tcc/libtccadd.so
ldd build/tcc/hello
```

检查目标：

```text
ELF 头合法
动态段来自系统 glibc / 动态加载器
存在我们生成的 .text/.data/.bss
没有 gcc 包装脚本参与最终链接
```

### 1.6 编译器级信任模型

目标不是重写整个 Linux 生态，而是守住关键路径：

```text
源码 -> C1/TCC 前端 -> 机器码 -> 我们的 ELF 链接 -> 最终二进制
```

系统头文件、crt、glibc、binutils 可以作为固定输入，最终 ELF 由我们自己的
链接器产生。

必须承认：

```text
源码开源，只能证明源码可见
不能自动证明运行中的编译器二进制来自这份源码
```

编译器可以在生成目标代码时注入后门，而源码本身看起来完全正常。这就是
“Trusting Trust”问题。因此需要以下约束：

```text
1. 小 TCB
   C1/TCC 比 GCC 小得多，审计成本更低。

2. 自举一致性
   C1 stage1 == stage2
   TCC self1 == self2
   这证明构建稳定，但不能单独证明没有后门。

3. 可复现构建
   同一输入、同一 flags 构建两次，产物哈希一致。
   不嵌入时间、用户名、绝对路径、随机 build-id。

4. 多样化双编译 DDC
   用两个独立编译器链构建同一个编译器或同一目标，
   比较产物/行为，避免单一编译器自说自话。

5. 输入固定
   sysroot、libc、crt、headers、binutils 的版本和哈希固定。

6. 构建清单
   记录编译器版本、输入哈希、flags、输出哈希。

7. 无隐藏代工
   最终 ELF 不允许偷偷调用 gcc/ld 生成；
   gcc 只允许作为 bootstrap 工具。

8. 干净环境
   构建不联网，不读取未记录路径，不执行构建期可疑代码。
```

结论：

```text
开源解决“代码可检查”
可复现构建解决“二进制是否来自这份代码”
DDC 解决“是否只有一个编译器自说自话”
小编译器解决“是否有人能审计完”
```

当前仍是：

```text
TCC、C1、BusyBox 补丁、系统 libc/crt 输入需要持续审计
seed.elf 二进制根仍是剩余信任根
```

因此不能宣称“谁都黑不了”。更准确的是：

```text
攻击面比 GCC 小
信任链比普通二进制分发清楚
但仍需可复现构建、DDC 和输入固定来闭合证明。
```

---

## 2. 版本策略

### 2.1 核心判断

```text
GCC 4.7.4 是最后一代主要用 C 实现的 GCC。
GCC 4.8 开始要求 C++ host compiler。
```

因此版本链必须从 GCC 4.7.4 开始，不能直接跳到现代 GCC。

### 2.2 第一阶段固定版本

| 组件 | 版本 | 作用 |
| --- | --- | --- |
| TCC | 0.9.27 | 当前自举编译器 |
| GCC | 4.7.4 | 第一个 GCC 目标 |
| binutils | 2.24 | 初期 as/ld |
| glibc | 2.17 | 旧 sysroot 基线 |
| GMP | 4.3.2 | GCC 4.7 依赖 |
| MPFR | 2.4.2 | GCC 4.7 依赖 |
| MPC | 0.8.1 或 1.0.1 | GCC 4.7 依赖 |
| make | 宿主 make | 构建驱动 |

### 2.3 第二阶段版本阶梯

以下版本是候选阶梯，每一级必须单独验证，不跨级跳：

```text
GCC 4.7.4
  -> GCC 4.8.5
  -> GCC 4.9.4
  -> GCC 5.5.0
  -> GCC 6.5.0
  -> GCC 7.5.0
  -> GCC 8.5.0
  -> GCC 9.5.0
  -> GCC 10.5.0
  -> GCC 11.5.0
  -> GCC 12.3.0
```

实际能跳到哪一级取决于中间版本对 C++ 标准、glibc 和 binutils 的要求。

### 2.4 源码目录建议

```text
$BIT/tcc-0.9.27/           TCC fork 基线，已在仓库
$SRC/gcc-4.7.4/            GCC 源码，仓库外
$SRC/binutils-2.24/
$SRC/glibc-2.17/
$SRC/gmp-4.3.2/
$SRC/mpfr-2.4.2/
$SRC/mpc-0.8.1/
$BUILD/gcc47/              GCC 4.7 out-of-tree 构建目录
```

GCC/binutils/glibc 源码体积大，不建议 vendor 进 bit 主仓库。

---

## 3. 总体架构

```text
C1
  -> bit-tcc
  -> GCC 4.7.4 cc1
  -> GCC 4.7.4 自举
  -> GCC 4.7.4 g++ / cc1plus
  -> GCC 4.8.5
  -> 后续 GCC 阶梯
  -> binutils + glibc + Linux 用户态程序
```

职责：

```text
C1        最底层可信自举
bit-tcc   轻量 C 编译器，负责把 GCC 4.7 C 前端拉起来
GCC 4.7   完整 GNU C 编译器，开始接管 GNU 生态
GCC 4.8+  逐步进入 C++ 和现代 Linux 生态
```

---

## 4. 组件级推进原则

不要直接：

```sh
make CC=bit-tcc
```

应当：

```text
一个组件 -> 一个文件 -> 一个对象
每层通过后再进入下一层
```

每个组件只回答三个问题：

```text
1. bit-tcc -c 能否生成合法 .o
2. 系统 ld 能否链接该 .o
3. 行为是否与 host gcc 参考结果一致
```

初期只让 TCC 做 `-c`，链接继续交给系统 binutils。

---

## 5. GCC 4.7.4 组件顺序

### 5.1 libiberty

先编译：

```text
cplus-dem.c
argv.c
obstack.c
hashtab.c
splay-tree.c
fibheap.c
pex-unix.c
```

目标：

```text
bit-tcc 能稳定生成 libiberty 对象
```

当前状态：libiberty 和 libcpp 都已通过。入口：

```sh
./tests/gcc47/run.sh
```

已编译：

```text
libiberty: cplus-dem.c argv.c obstack.c hashtab.c splay-tree.c fibheap.c pex-unix.c
libcpp:    lex.c macro.c directives.c expr.c files.c identifiers.c init.c
           line-map.c symtab.c charset.c traditional.c pch.c mkdeps.c errors.c
gcc:       ggc-page.c bitmap.c vec.c double-int.c tree.c diagnostic.c
x86:       i386.c i386-c.c
C front:   c-decl.c c-parser.c c-typeck.c c-convert.c c-lang.c
c-family:  c-common.c c-format.c c-lex.c c-cppbuiltin.c c-opts.c
           c-pragma.c c-semantics.c
```

这是 GCC 4.7.4 的前四层兼容验证。

cc1 链接和运行已通过：

```sh
./tests/gcc47/cc1.sh
```

结果：

```text
PASS: TCC-linked GCC 4.7.4 cc1 compiles and runs hello
```

cc1 由 bit-tcc 链接；它使用动态 MPFR/MPC，需要
`LD_LIBRARY_PATH=/opt/gcc47-deps/lib`。

xgcc 驱动也已由 bit-tcc 链接，并能编译/运行 hello：

```sh
./tests/gcc47/xgcc.sh
```

结果：

```text
PASS: TCC-linked GCC 4.7.4 xgcc compiles and runs hello
```

xgcc 当前仍使用系统 libgcc/crtbegin 来完成 libgcc 链接。

TCC 构建的 xgcc 已能再编译 GCC C 前端源码：

```sh
./tests/gcc47/selfhost.sh
```

结果：

```text
PASS: TCC-built GCC 4.7.4 xgcc compiles GCC C frontend sources
```

这是“TCC 构建的 GCC 再编译 GCC”的自举探针。

stage2 cc1 已成功构建并运行：

```sh
./tests/gcc47/stage2.sh
```

结果：

```text
PASS: GCC 4.7.4 stage2 cc1 built by TCC-built stage1 GCC
```

边界：

```text
stage1 cc1/xgcc 由 TCC 链接
stage2 cc1 由 stage1 GCC 编译
generator 使用系统 gcc
最终 stage2 链接使用 stage1 GCC/collect2
```

自有 libgcc 已构建并可用：

```sh
./tests/gcc47/libgcc.sh
```

结果：

```text
PASS: GCC 4.7.4 libgcc built and linked by stage1 GCC
```

为兼容现代 glibc，`libgcc/config/i386/linux-unwind.h` 使用 `ucontext_t`，
补丁为 `tests/gcc47/libgcc-ucontext.patch`。

stage2 xgcc 驱动也已用 stage1 xgcc 构建，并使用自有 libgcc：

```sh
./tests/gcc47/driver.sh
```

结果：

```text
PASS: GCC 4.7.4 stage2 xgcc uses its own libgcc
```

stage3 cc1 也已构建；stage2 与 stage3 对同一源码生成相同汇编：

```sh
./tests/gcc47/stage3.sh
```

结果：

```text
PASS: GCC 4.7.4 stage2 and stage3 cc1 produce identical assembly
```

GCC 4.8.5 已开始接力：

```sh
./tests/gcc48/run.sh
```

结果：

```text
PASS: GCC 4.8.5 cc1 and cc1plus compile C and C++ hello
```

GCC 4.8 由系统 g++ 构建，需 `-std=gnu++98` 兼容现代 g++。自有 GCC 4.7
尚未提供 C++ 前端，因此 4.8 仍是系统 C++ 桥接。

GCC 4.8 的 libstdc++ 也已构建，C++ `std::string`/`iostream` 程序可运行：

```sh
./tests/gcc48/libstdcxx.sh
```

结果：

```text
PASS: GCC 4.8.5 C++ program runs with built libstdc++
```

GCC 4.8 libgcc 的现代 glibc 兼容补丁为
`tests/gcc48/libgcc-ucontext.patch`。GCC 4.9.4 已开始：

```sh
./tests/gcc49/run.sh
```

结果：

```text
PASS: GCC 4.9.4 cc1 and cc1plus compile C and C++ hello
```

GCC 5.5.0 已开始：

```sh
./tests/gcc55/run.sh
```

结果：

```text
PASS: GCC 5.5.0 cc1 and cc1plus compile C and C++ hello
```

GCC 7.5.0 已开始：

```sh
./tests/gcc75/run.sh
```

结果：

```text
PASS: GCC 7.5.0 cc1 and cc1plus compile C and C++ hello
```

完整现代 GCC 阶梯仍未完成。

### 5.2 libcpp

```text
lex.c
macro.c
directives.c
expr.c
files.c
identifiers.c
init.c
line-map.c
symtab.c
```

目标：

```text
TCC 能处理 GCC 自己的预处理逻辑
```

### 5.3 GCC 基础设施

```text
ggc-*.c
bitmap.c
vec.c
double-int.c
tree.c
diagnostic.c
```

目标：

```text
GCC 内存管理、树、诊断、宽整数基础可用
```

### 5.4 x86_64 后端

```text
config/i386/*.c
```

目标：

```text
TCC 能编译 GCC 的 x86_64 机器描述代码
```

### 5.5 C 前端

```text
c-decl.c
c-parser.c
c-typeck.c
c-convert.c
c-common.c
c-format.c
c-lex.c
```

目标：

```text
生成可运行的 cc1
```

### 5.6 cc1 链接

初期：

```text
cc1 对象由 bit-tcc 生成
cc1 使用系统 ld / libc 链接
```

不要求：

```text
TCC 自己链接 cc1
TCC 自己生成 libgcc
```

---

## 6. GCC 4.7.4 配置方案

第一轮使用 host gcc 完成 configure，生成：

```text
config.h
auto-host.h
Makefile
```

然后只把 C 编译任务替换为 bit-tcc。

建议 configure：

```sh
../gcc-4.7.4/configure \
  --build=x86_64-linux-gnu \
  --host=x86_64-linux-gnu \
  --target=x86_64-linux-gnu \
  --disable-bootstrap \
  --enable-languages=c \
  --disable-multilib \
  --disable-libgomp \
  --disable-libssp \
  --disable-libquadmath \
  --disable-libatomic \
  --disable-werror \
  --prefix=/opt/gcc47
```

构建目标：

```sh
make all-gcc
```

不要第一步就做完整 `make`。

首轮环境：

```text
CC=bit-tcc
system as
system ld
system glibc
system GMP/MPFR/MPC
```

---

## 7. GCC 4.7.4 自举验收

### 7.1 第一层：能生成 cc1

```text
bit-tcc 编译 GCC 4.7 C 源文件
系统 ld 链接出 cc1
cc1 能编译 hello.c
```

### 7.2 第二层：GCC 自举

```text
stage1 cc1 编译 GCC 4.7 -> stage2 cc1
stage2 cc1 编译 GCC 4.7 -> stage3 cc1
stage2 == stage3
```

### 7.3 第三层：GNU 组件

```text
GCC 4.7 编译 libiberty
GCC 4.7 编译 binutils 2.24
GCC 4.7 编译 glibc 2.17
```

只有这三层都通过，才叫 GCC 4.7 自举完成。

---

## 8. TCC 兼容矩阵

### 8.1 已有基础

```text
__attribute__ token 级消费
packed / aligned
__extension__
__builtin_expect
__builtin_constant_p
__builtin_unreachable
unsigned char bit-field
匿名 struct/union 展平
char* / unsigned char* 兼容
void* / 函数指针兼容
```

### 8.2 GCC 4.7 前必须补

```text
__int128
完整 __builtin_* 集合
GNU statement expression
typeof
__thread / TLS
完整 __attribute__ 语义
GNU inline asm
复杂位域
```

### 8.3 第二阶段再补

```text
vector extensions
嵌套函数
C++ 前端
异常 / RTTI / 模板
```

原则：

```text
先补 GCC 4.7 C 前端真正用到的能力
不要为了“完整 GNU C”无限扩张
```

---

## 9. TCC fork 维护策略

### 9.1 基线

```text
TCC 0.9.27 原始源码
```

### 9.2 fork

```text
分支：bit-tcc
基线标签：tcc-0.9.27
```

### 9.3 修改位置

```text
tccpp.c       预处理、宏、include
tccgen.c      语法、类型、内建、代码生成
tcc.h         类型和编译状态
x86_64-gen.c  x86_64 代码生成
```

### 9.4 不直接修改

```text
/opt 中的临时源码
构建产物中的源码副本
不可追踪的 patch
```

### 9.5 补丁组织

如果暂不建独立仓库，至少要有：

```text
tcc-0.9.27/
patches/
  attribute.patch
  builtins.patch
  int128.patch
  tls.patch
  asm.patch
```

每个补丁单独可回滚，不混成一个大提交。

### 9.6 许可证

TCC 使用 LGPL。fork、修改、分发时必须保留版权和许可证文本。

---

## 10. C++ 过渡方案

TCC 不负责 C++ 前端。

GCC 4.8 起需要 C++ host，建议：

```text
GCC 4.7.4 C 前端
  -> 系统 g++ 构建 GCC 4.7.4 cc1plus
  -> GCC 4.7.4 g++ 构建 GCC 4.8.5
  -> GCC 4.8.5 g++ 构建 GCC 4.9.4
  -> GCC 4.9.4 构建 GCC 5.5.0
```

明确边界：

```text
TCC 只负责 C
GCC 4.7 g++ 负责 C++ 桥接
后续 GCC 负责现代 C++
```

---

## 11. binutils / glibc

第一阶段：

```text
binutils 2.24
glibc 2.17
```

用途：

```text
提供旧 sysroot
避免现代 glibc 头文件直接压垮 GCC 4.7
提供 as/ld 的已知基线
```

第二阶段：

```text
GCC 5/7/9
较新的 binutils
较新的 glibc
```

注意：

```text
编译器自举和用户态生态是两条不同的验收线。
GCC 能自举，不等于能直接构建现代发行版。
```

---

## 12. Linux 生态验收

### 12.1 C 生态

```text
GCC 4.7.4
binutils 2.24
glibc 2.17
BusyBox
Lua
musl 小子集
```

### 12.2 现代生态

```text
GCC 7.5 / 9.5 / 12.3
现代 binutils
现代 glibc
Autotools / Make / CMake
真实 Linux 用户态项目
```

### 12.3 验收标准

```text
能编译
能链接
能运行
能重复构建
能自举
```

不是“编译通过”就算完成。

### 12.4 C/C++ 项目适配

必须分成两条工具链，而不是让 TCC 直接支持 C++：

```text
C 项目   -> bit-tcc
C++ 项目 -> GCC 4.7.4+ g++ / libstdc++
```

原因：

```text
TCC 0.9.27 没有 C++ 前端
没有模板、异常、RTTI、命名空间、C++ ABI
没有 libstdc++、__cxa_*、crtbegin/crtend
```

因此 C++ 适配不是改 TCC 语法，而是把 GCC C++ 工具链接进来。

#### C 项目适配

```text
libc headers / sysroot
-c / -o
-L / -l
-shared
-rpath / RUNPATH
pkg-config
Make / CMake / Autotools
```

第一阶段目标：

```text
BusyBox
Lua
小型 C 库
```

#### C++ 项目适配

```text
GCC 4.7.4 cc1plus
libstdc++
libgcc_s
crtbegin.o / crtend.o
__cxa_* 异常和 RTTI
动态加载器
```

构建顺序：

```text
系统 g++ 构建 GCC 4.7.4 cc1plus/libstdc++
  -> GCC 4.7.4 g++ 构建 GCC 4.8.5
  -> GCC 4.8.5 g++ 构建 GCC 4.9.4
  -> 后续 GCC
```

C++ 标准边界：

```text
GCC 4.7   C++98 / 部分 C++11
GCC 4.8   C++11 改善
GCC 5+    C++11 更完整
现代项目  往往需要 C++14/17/20，必须继续升级 GCC
```

驱动层建议：

```text
bitcc     -> C 项目，走 bit-tcc
bitc++    -> C++ 项目，走 GCC g++
```

不要把两个前端硬塞进一个不完整的 C++ 实现里。

---

## 13. 测试与门禁

### 13.1 当前门禁

```sh
./tests/tcc/upstream.sh
./build.sh
HOSTED=1 ./build.sh
```

### 13.2 GCC 4.7 门禁

建议新增：

```text
tests/gcc47/libiberty.sh
tests/gcc47/libcpp.sh
tests/gcc47/cc1.sh
tests/gcc47/selfhost.sh
```

每个脚本输出：

```text
PASS / FAIL / SKIP
失败文件
失败阶段
首个错误
对象是否可链接
```

### 13.3 固定基线

```text
TCC 0.9.27
GCC 4.7.4
binutils 2.24
glibc 2.17
```

不要在不同阶段混用多个 GCC 版本。

---

## 14. 风险

| 风险 | 影响 | 缓解 |
| --- | --- | --- |
| TCC 不支持 `__int128` | GCC 4.7 大量文件失败 | 先做最小 int128 支持 |
| TCC 不支持 GNU asm | 后端文件失败 | 初期绕开或补最小 asm |
| GCC configure 依赖 host gcc | 无法纯 TCC 启动 | 先 host configure，再替换 CC |
| GCC 4.7 与现代 glibc 不兼容 | 头文件/ABI 失败 | 固定 glibc 2.17 sysroot |
| GCC 4.8 需要 C++ | TCC 无法直接跨级 | GCC 4.7 g++ 过渡 |
| 旧 GCC 构建链缺 GMP/MPFR/MPC | configure 失败 | 固定旧版本依赖 |
| 修改 TCC 破坏 TCC 自举 | 自举链断裂 | 每补丁重跑 tests/tcc/upstream.sh |
| 直接用系统头文件 | 结果不可复现 | 固定 sysroot/include 基线 |

---

## 15. 里程碑

### M0：TCC 自举

状态：已完成。

```text
C1 -> TCC
TCC -> TCC
```

### M1：TCC 组件 probe

目标：

```text
TCC 编译 libiberty
```

### M2：GCC 4.7 cc1

目标：

```text
TCC 编译 GCC 4.7 C 前端
生成可运行 cc1
```

### M3：GCC 4.7 自举

目标：

```text
GCC 4.7 编译自身
stage2 == stage3
```

### M4：C++ 过渡

目标：

```text
GCC 4.7 g++ 构建 GCC 4.8+
```

### M5：现代 GCC

目标：

```text
GCC 7/9/12 逐级自举
```

### M6：Linux 生态

目标：

```text
BusyBox / Lua / musl / 真实用户态项目
```

---

## 16. 第一周执行清单

```text
[ ] 建 bit-tcc fork 或补丁目录
[ ] 固定 TCC 0.9.27 基线
[ ] 下载 GCC 4.7.4 源码到 $SRC
[ ] 用 host gcc 跑 GCC 4.7 configure
[ ] 生成 tests/gcc47/libiberty.sh
[ ] 用 bit-tcc 编译 cplus-dem.c
[ ] 用 bit-tcc 编译 argv.c
[ ] 用 bit-tcc 编译 obstack.c
[ ] 记录首个 TCC 错误
[ ] 修 TCC 或补 shadow header
[ ] 重跑 ./tests/tcc/upstream.sh
```

第一周只追求：

```text
bit-tcc 能稳定生成 libiberty 对象
```

不要在第一周尝试完整 GCC。

---

## 17. 一句话路线

```text
TCC 不是直接吞下任意 GCC；
TCC 先吞下 GCC 4.7.4 的 C 前端，
让 GCC 4.7.4 自举，
再由 GCC 4.7.4 接力进入 GCC 4.8+ 和现代 Linux 生态。
```
