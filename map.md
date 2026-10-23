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
bit-tcc -B runtime -o hello examples/hello.c
bit-tcc -B runtime -shared -o libtccadd.so tcc-lib.c
bit-tcc -B runtime -o tcc-app tcc-app.c -L. -ltccadd
```

依赖边界：

```text
TCC 自己编译 libtcc1
TCC 自己完成 ELF 链接
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
