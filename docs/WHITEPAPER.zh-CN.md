# 从种子到 GCC 14.2

## 一条可复现的编译器自举链

版本：2026-10  
状态：工程验证稿  
对应提交：`83b6f41`

---

## 摘要

本文描述 BitKindle Native 的 GCC 自举天梯：从十六进制机器码种子出发，经过 A0/A1 装配器、C0 过渡编译器、C1 编译器、TCC 0.9.27，最终推进到 GCC 14.2。

这不是一条“用现成 GCC 编译新 GCC”的普通构建链。项目的核心问题是：

> 当最底层只剩一个可审计的 seed 时，编程世界能否被一级一级重新推导出来？

当前已经完成并验证的部分包括：

```text
seed -> A0 -> A1 -> C0 -> C1
C1 -> TCC 0.9.27 -> TCC self-host
TCC -> GCC 4.7 pass1 -> GCC 4.7 pass2
GCC 4.7 -> 4.8 -> 4.9 -> 5.5 -> 7.5 -> 9.5
         -> 12.3 -> 13.3 -> 14.2
```

其中 GCC 天梯由 `tests/gcc-ladder/run.sh` 驱动，已在干净构建目录中一次跑通。每一级都构建 `all-gcc`、`all-target-libgcc`、`all-target-libstdc++-v3`，并编译运行 C++ iostream 测试。

GCC 4.7 的 configure 内部仍按自身逻辑执行 `AC_PATH_PROG`，会在日志里发现 `/usr/bin/gcc` 等路径；这是 GCC 4.7 构建系统写死的探测，不参与目标构建。实际 Makefile 中 `CC/CXX` 指向 TCC，`CC_FOR_TARGET/GCC_FOR_TARGET` 指向当前 stage 的 xgcc。GCC 4.7 pass1 的 `CC/CXX` 都是 TCC；pass1 生成的 `cc1plus` 提供 C++ 能力，pass2 用这份能力重建 GCC 4.7，GCC 4.8 及以后则完全由前一级 4.7 的 C++ 能力承接。完整 `make install`、逐级 GCC testsuite、C1 -> TCC 与 GCC 天梯的统一 driver、自有 binutils/glibc/sysroot 尚未完成。`seed.elf`/`seed.hex` 仍是剩余信任根。

---

## 1. 第一推动力：编程世界如何被重新推导出来

### 1.1 一个工程问题

每个深入写过编译器、链接器或操作系统接口的人，最终都会遇到同一个问题：

```text
编译器从哪里来？
操作系统从哪里来？
如果现有工具链全部消失，还剩下什么可以信任？
```

这不是哲学修辞，而是构建系统必须回答的工程问题。任何自举链都必须有一个起点。起点之上的每一级，要么由前一级生成，要么由外部输入提供。所谓“从零自举”，并不是真的没有信任根，而是把信任根压缩到最小、可审计、可复现的形态。

### 1.2 从机器码到符号

最早的计算机只认识机器码。程序员直接面对指令编码、寄存器和内存地址。存储程序计算机把“程序”变成可以存放在内存中的数据，这是第一次抽象：代码不再是物理接线，而是可加载的字节序列。

随后出现汇编器。汇编器把助记符、标号和地址表达式翻译成机器码；链接器把多个目标文件中的符号引用与定义连接起来；加载器把可执行文件映射到进程地址空间。这个阶段确立了一个持续至今的分层：

```text
源码
  -> 编译器/汇编器
  -> 目标文件
  -> 链接器
  -> 可执行文件
  -> 加载器
  -> 运行时与系统调用
```

每一层都依赖下一层的契约。ELF 的节表、符号表、重定位类型、ABI 寄存器规则、启动代码、系统调用号，都是这条链上的接口。

### 1.3 高级语言与编译器

高级语言把机器细节隐藏在语言语义之后。编译器负责把语言语义下降到机器指令。现代编译器通常分为前端、优化器和后端：

```text
前端：词法、语法、类型检查、语义分析
优化器：中间表示上的分析与变换
后端：指令选择、寄存器分配、调用约定、目标文件生成
运行时：启动代码、异常、内存管理、语言运行时库
```

GCC 的重要之处不只是“能编译 C”，而是它把前端、优化器、后端、目标库和驱动组织成一套可自举的构建系统。GCC 4.7 的 C 前端 `cc1` 和 C++ 前端 `cc1plus` 本身由 C 源码实现；GCC 4.8 起，编译器主体逐渐转向 C++ 实现。这个语言边界直接影响自举策略。

### 1.4 自举与固定点

自举的核心是固定点：

```text
stage0：外部编译器编译出第一版自身
stage1：stage0 编译同一份源码
stage2：stage1 再编译同一份源码
要求：stage1 与 stage2 在约定范围内一致
```

固定点不是“看起来一样”，而是可验证的一致性：逐字节相同的二进制、相同的目标文件、相同的汇编输出，或者经过定义的可复现差异。C1 当前采用 `stage1 == stage2` 的逐字节比较；GCC 天梯则使用逐级构建和 C++ 运行测试作为工程门禁。

### 1.5 信任根与 Thompson 问题

Thompson 在 *Reflections on Trusting Trust* 中指出：即使源码看起来干净，编译器二进制也可能携带不可见的后门；用被污染编译器编译“干净源码”，后门仍可能被复制到新编译器中。

因此，自举链的安全目标不是“没有信任根”，而是：

```text
信任根尽量小
信任根有哈希和来源记录
每一级都能从文本/源码重新生成
关键产物能逐字节比较
构建过程尽量可复现
```

本项目的 `seed.hex`、`seed.elf`、A0/A1 的 hex 物化流程、C1 的 `stage1 == stage2`，都是围绕这个目标设计的。剩余信任根仍然是 seed 本身；白皮书不会把“自举完成”写成“信任问题消失”。

### 1.6 本项目在演化链中的位置

本项目把上述历史压缩成一条可以执行的因果链：

```text
机器码 seed
  -> A0：从 seed 物化，解码/装配 A1
  -> A1：装配器
  -> C0：过渡编译器
  -> C1：Linux SysV 自举编译器
  -> TCC 0.9.27
  -> TCC self-host
  -> GCC 4.7 pass1 / pass2
  -> GCC 4.8 ~ 14.2
```

这条链的每一级都不是装饰。A0/A1 解决“如何从机器码生成可运行工具”；C0 解决“如何把受限 C 推到 C1”；C1 解决“如何在没有外部 C 编译器时完成自身编译和链接”；TCC 解决“如何用更小的 C 编译器承接生态”；GCC 天梯解决“如何从 TCC 的能力边界推进到现代 GCC”。

---

## 2. 目标、威胁模型与边界

### 2.1 目标

```text
1. 从可审计 seed 重建 C1。
2. C1 能编译自身，stage1 == stage2。
3. C1 能编译 TCC，TCC 能自举。
4. TCC 能构建 GCC 4.7 的 C/C++ 前端和目标库。
5. GCC 4.7 能重建自身，并继续推进到 GCC 14.2。
6. 每一级构建都有日志、测试和可复现入口。
```

### 2.2 威胁模型

```text
T1 宿主编译器污染目标代码
T2 旧构建目录/缓存残留导致隐式复用
T3 静态库或目标文件混入不同 ABI 的 varargs/类型系统
T4 二进制后门通过编译器自举传播
T5 不可复现的构建差异掩盖 miscompile
T6 把“configure 能用 host”误写成“目标代码由 host 构建”
```

对应措施包括：独立 build tree、显式 `CC/CXX/CC_FOR_BUILD`、build-tree wrapper、逐级日志、源码补丁、C++ hello、C1 固定点比较、seed 哈希与 hex 物化。

### 2.3 非目标

```text
第一阶段不消灭系统 as/ld/glibc/crt
第一阶段不重写 GCC 4.7 configure 自带的 `AC_PATH_PROG` 探测；其探测结果不参与目标构建
第一阶段不承诺每一级 GCC 都逐字节可复现
第一阶段不把 C1 -> TCC 与 GCC 天梯合并成单一 driver
```

这些不是“已完成”的替代说法，而是明确的剩余工作。

---

## 3. 从 seed 到 C1

### 3.1 两条路线

项目同时保留两条启动路线：

```text
A. gcc stage0 路线
   gcc/clang -> C1 stage0 -> C1 stage1 -> C1 stage2

B. 纯种子路线
   seed.elf -> A0 -> A1 -> C0 -> C1
```

A 路线是工程启动路径，允许平台原生 C 编译器作为 stage0。B 路线是“第一推动力”路径，目标是让 C1 的来源不依赖现成 C 编译器。

### 3.2 纯种子链

Linux 纯种子链的构件包括：

```text
bootstrap/linux/seed.S       手写十六进制解码器
bootstrap/linux/seed.hex     seed.elf 的原始十六进制
bootstrap/linux/seed.elf     可复现的种子可执行文件
bootstrap/linux/a0.seed.hex  A0 的按需物化输入
bootstrap/linux/a1.seed.hex  A1 的按需物化输入
C0                           过渡编译器
C1                           SysV Linux 编译器
```

基本关系：

```text
seed.hex -> seed.elf
seed.elf -> A0（从 a0.seed.hex 物化）
A0 -> A1（从 a1.seed.hex 物化）
A1 -> 汇编/装配
C0 -> 受限 C -> A1 文本
C0 -> bootstrap.c -> capacity -> C1
```

A0/A1 二进制不提交，按需从 hex 物化；`seed.hex` 可复现 `seed.elf`。这使 seed 及其上游生成链可以被审计，而不是只留下一个无法解释的二进制。

### 3.3 C0 与 C1

最小 freestanding C0 已能把受限 C 编译成 A1 文本，并支持函数、局部变量、控制流、表达式和全局变量。C0 能把 `bootstrap.c` 编成 A1，再由 A1 装配成 ELF。C0v2 直接从 A1 生成 Linux C1，使原生 C1 能编译 `compiler.c`/`runtime.c`。

C1 的关键结果：

```text
目标：SysV AMD64 / ELF64
编译器自身：freestanding
链接：c1link
stage1 == stage2：逐字节一致
原生 linker：能重新链接出逐字节相同的二进制
```

C1 的原生链接闭环意味着：构建 C1 不再必须依赖系统 `ld` 来完成最终链接。hosted glibc 测试仍使用系统 `ld`，这是测试路径与 freestanding 自举路径的边界。

### 3.4 信任根

当前剩余信任根是：

```text
seed.hex / seed.elf
以及生成 A0/A1 所需的一次性工具链
```

`seed.elf` 是二进制根，不能因为“源码可读”就宣称它不存在。白皮书的正确表述是：

```text
信任根已缩小并可复现，但没有被消灭。
```

---

## 4. C1 到 TCC

### 4.1 C1 编译 TCC 0.9.27

C1 已能编译 TCC 0.9.27，产出 `build/tcc/bit-tcc`。TCC 随后完成自举，并构建 `libtcc1.a`、`va_list.c`、`alloca86_64.S` 等运行时构件。

验证分为两层：先证明 C1 能编出 TCC、TCC 能编出自身，再证明这个 TCC 能编译真实用户态项目。

| 阶段 | 产物 | 验证入口 | 结果 |
| --- | --- | --- | --- |
| C1 编译 TCC | `build/tcc/bit-tcc` | `tests/tcc/upstream.sh` | PASS |
| TCC 自举 | `build/tcc/tcc-stage2` | `tests/tcc/upstream.sh` | PASS |
| TCC 编译 BusyBox 1.35 最小配置 | `busybox_unstripped` | `tests/tcc/busybox.sh` | PASS |
| TCC 完整构建 BusyBox 1.36.1 defconfig（带维护补丁） | `busybox_unstripped` | `tests/busybox/run.sh` | 编译、链接、运行 PASS |
| TCC 编译 Lua 5.4.7 | `lua` / `liblua.a` | `tests/lua/run.sh` | PASS |
| TCC 编译 cJSON 1.7.18 | 测试程序 | `tests/cjson/run.sh` | PASS |
| TCC 编译 SQLite 3.45.3 | 测试程序 | `tests/sqlite/run.sh` | PASS |

BusyBox 1.36.1 使用 `tests/busybox/busybox-1.36.1-tcc.patch`，处理现代 kernel headers 的 CBQ 定义和 `__dso_handle`；完整 `defconfig` 已由 TCC 编译、链接并运行通过。C 源由 TCC 编译和链接，少量 `.S` 汇编源由系统 as 处理。Lua 5.4.7、cJSON 和 SQLite 使用无补丁源码构建。

这一阶段的意义是：TCC 不只是一个能编译自身的小编译器，它还能编译 BusyBox、Lua 5.4.7、cJSON、SQLite 这类真实 C 项目。它不是最终目标，而是从 C1 走向 GCC 的桥梁。

### 4.2 TCC 的边界

TCC 能编译 C，但不能直接编译 C++。GCC 4.7 的 C++ 前端 `cc1plus` 是 C 源码实现的，因此 TCC 可以编译 `gcc/cp/*.c` 得到 `cc1plus`；但 `libstdc++-v3` 的运行时源码是 C++，必须由 `cc1plus` 或后续 GCC 编译。

这个边界必须写清楚：

```text
cc1plus 实现：C 源码，可由 TCC 编译
libstdc++：C++ 源码，不能由 TCC 直接编译
```

---

## 5. TCC 到 GCC 4.7

### 5.1 两遍自举

GCC 4.7 的 TCC 起始链采用两遍：

```text
Pass 1：TCC 构建 GCC 4.7
  CC=TCC, CXX=TCC
  -> cc1
  -> TCC 编译 gcc/cp/*.c -> cc1plus
  -> cc1plus 编译 libstdc++
  -> TCC-built target libgcc/libstdc++

Pass 2：第一遍 GCC 4.7 重建 GCC 4.7
  CC/CXX = pass1 build-tree compiler
  -> stage2 GCC 4.7
```

Pass 2 的作用是移除 4.8 过渡对 TCC 和外部 C++ 编译器的依赖。它不声称 pass1 与 pass2 逐字节相同；GCC 构建本身包含时间戳、路径和配置差异。可验证门禁是：每一遍都能构建目标库、编译并运行 C++ 程序。

因此 4.7 阶段的 C++ 能力来源是：

```text
TCC 编译 gcc/cp/*.c -> pass1 cc1plus
pass1 cc1plus 编译 libstdc++
pass1 GCC 4.7 重建自身 -> pass2 cc1plus/libstdc++
pass2 GCC 4.7 的 gcc/g++ -> GCC 4.8
```

这条链的目标 C++ 编译器来自自举链；configure 的路径探测不改变目标构建。

### 5.2 类型系统扩展

TCC 0.9.27 原始类型系统不足以处理 GCC 4.7 soft-fp 所需的 128 位模式。项目内 TCC fork 增加了：

```text
mode(DI/TI/TF/SF/DF/XF)
__mode__(__TI__/__TF__)
typedef 后置 attribute
VT_QLONG -> integer ABI
VT_QFLOAT -> SSE ABI
```

对应修改位于 `tcc-0.9.27/tccgen.c`、`tcctok.h`、`x86_64-gen.c`。这些修改使 TCC 能解析 GCC 4.7 的 `__attribute__((mode(...)))`，并正确分类 x86_64 SysV ABI 中的 128 位整数和浮点类型。

#### 5.2.1 SysV AMD64 传参契约

`mode(TI)` 和 `mode(TF)` 都是 16 字节对象，在 SysV AMD64 中分别属于 INTEGER 和 SSE class。ABI 的关键不是类型名字，而是：

```text
class（INTEGER / SSE / MEMORY）
size（8 字节为一个 eightbyte）
可用寄存器数量
```

TCC 的 `classify_x86_64_arg()` 根据 `type_size()` 和 `classify_x86_64_inner()` 计算 `reg_count`：

```text
size > 16                 -> MEMORY，整对象走栈
INTEGER class, size > 8   -> reg_count = 2，ret_t = VT_QLONG
SSE class, size > 8       -> reg_count = 2，ret_t = VT_QFLOAT
```

调用者侧，`gfunc_call()` 先做从左到右的分配预扫描：

```text
INTEGER：nb_reg_args + reg_count <= 6 才走 GPR
SSE：    nb_sse_args + reg_count <= 8 才走 XMM
否则整个参数走栈，不允许半个 128 位对象拆成寄存器 + 栈
```

寄存器映射遵循 SysV：

```text
INTEGER 参数：RDI, RSI, RDX, RCX, R8, R9
SSE 参数：    XMM0 ... XMM7
INTEGER 返回：RAX:RDX
SSE 返回：    XMM0:XMM1
```

在实现上：

```text
VT_QLONG  -> x86_64_mode_integer -> 两个连续 GPR
VT_QFLOAT -> x86_64_mode_sse     -> 两个连续 XMM
```

调用者用 `gen_reg`/`sse_reg` 从高到低扣减寄存器配额，`arg_prepare_reg()` 把逻辑索引映射到参数寄存器；由于准备参数时 `gv()` 可能破坏 RDX/RCX，索引 2/3 暂存到 R10/R11，最后再统一搬到 RDX/RCX。128 位 INTEGER 用 `r` 和 `vtop->r2` 两个源寄存器，SSE 则通过 `RC_FRET` 成对加载到 `xmm0/xmm1` 后再搬到目标 XMM 对。被调用者侧反向执行：INTEGER 用 `arg_regs[]` 连续保存，SSE 用 `sse_param_index` 连续保存。

这条规则保证 soft-fp 的 `TI`/`TF` 值在 TCC 调用 GCC 生成的代码时不会被错误地拆成“一个寄存器 + 一块栈”，也不会把 SSE class 误当 INTEGER class。

边界必须写明：当前验证覆盖了 GCC 4.7 soft-fp 构建、TCC-built libgcc/libstdc++ 和 C++ iostream 运行；尚未做完整的 SysV ABI 边界矩阵（寄存器耗尽、混合 INTEGER/SSE、可变参数、返回大对象等），因此不能把本节写成“ABI 全量形式化证明”。

### 5.3 soft-fp 与系统头补丁

GCC 4.7 的 target `libgcc` 依赖 soft-fp。TCC 构建需要 `tests/gcc47/tcc-softfp.patch`：

```text
libgcc/longlong.h            64-bit add/sub/mul/div/clz fallback
libgcc/config/i386/64/sfp-machine.h
                             关闭 TCC 不支持的 fenv inline asm
libgcc/soft-fp/op-common.h   TCC __FP_CLZ fallback
gcc/system.h                 limits/fcntl/stat/dlfcn/mman 等头
gcc/ggc-common.c / opts.c    TCC 下关闭 rlimit 相关代码
include/filenames.h          补 stddef.h
libiberty 相关头             补 stdlib/fcntl/stat 等
```

这些补丁解决的是“TCC 编译 GCC 4.7 源码”的系统头、limits 和 soft-fp 兼容问题，不是把 target libgcc 交给 host。

### 5.4 验证结果

第一遍已验证：

```text
all-gcc                  PASS
all-target-libgcc        PASS
all-target-libstdc++-v3  PASS
TCC-built cc1plus + libstdc++ + C++ iostream  PASS
```

第二遍已验证：

```text
GCC 4.7 pass2 all-gcc    PASS
GCC 4.7 pass2 libgcc     PASS
GCC 4.7 pass2 libstdc++  PASS
C++ iostream             PASS
```

因此，GCC 4.8 的起点不是 host GCC，而是 TCC-built、再由 GCC 4.7 重建的 stage2 4.7。

### 5.5 GCC 的 self-hosting 与链式自举

GCC 源码本身是 self-hosting 的：只要有一个可用的 C/C++ 编译器，GCC 就能编译自己的源码。这里的“self-hosting”描述的是源码能力，不是“从零启动”或“每个版本都必须执行 stage2/stage3”。

本项目的实际门禁是：

```text
GCC 4.7：
  TCC -> 4.7 pass1 -> 4.7 pass2
  这两遍是启动所必需的

GCC 4.8 ~ 14.2：
  前一级 GCC -> 下一级 GCC
  只要能构建下一级，并通过 all-gcc/libgcc/libstdc++/C++ iostream，
  就认为该级天梯通过
```

GCC 的 `make bootstrap` 会构建 stage1/stage2/stage3 并做一致性检查，但它是可选的验证流程，不是生成可用编译器的前提。`--disable-bootstrap` 下由上一代 GCC 构建出的编译器仍然是 self-hosting 的 GCC；本项目没有要求每个版本额外再编译自己一遍。

准确的结论是：

```text
GCC 源码是 self-hosting 的；
4.7 通过 TCC -> pass1 -> pass2 证明了启动路径；
4.8~14.2 由上一代 GCC 逐级构建；
不要求每个版本都额外执行自身 stage2。
```

---

## 6. GCC 4.8 到 14.2 天梯

### 6.1 驱动方式

`tests/gcc-ladder/run.sh` 采用 build-tree wrapper：

```text
前一级 build tree
  -> -B 指向其 gcc/
  -> -L 指向其 libstdc++/src/.libs
  -> C++ 编译使用 -x c++
  -> 链接补 -lstdc++
```

每一级只使用前一级的 build tree，不安装到 prefix，也不引用 host target 库。这样可以避免 `make install` 未完成带来的路径问题，同时保持目标库的来源清晰。

### 6.2 目标 triplet 与头文件

不同 GCC 版本生成的 target triplet 可能不同，例如：

```text
x86_64-unknown-linux-gnu
x86_64-pc-linux-gnu
```

驱动通过扫描 `*-linux-gnu` 目录确定当前 stage 的 triplet，并据此选择：

```text
<build>/<triplet>/libstdc++-v3/include
<build>/<triplet>/libstdc++-v3/include/<triplet>
<build>/<triplet>/libstdc++-v3/src/.libs
```

这避免了把 4.7 的头文件误用到 4.8/4.9 的 target libstdc++ 上。

### 6.3 C++ 标准策略

```text
4.7 pass2 / 4.8 all-gcc   gnu++98
4.7 / 4.8 libstdc++       gnu++11
4.9+ all-gcc              gnu++11
4.9+ libstdc++            gnu++11
```

GCC 12 的 libcody 在 configure 阶段要求精确 C++11，因此 12.3、13.3、14.2 也统一使用 `gnu++11`，而不是 `gnu++14/17`。

### 6.4 已验证结果

一次干净运行 `BUILD_BASE=/opt/gcc-ladder-final` 的输出：

```text
PASS: 4.7-pass1
PASS: 4.7-pass2
PASS: 4.8
PASS: 4.9
PASS: 5.5
PASS: 7.5
PASS: 9.5
PASS: 12.3
PASS: 13.3
PASS: 14.2
PASS: TCC -> GCC 4.7 (2 passes) -> GCC 14.2
```

每级测试内容：

```text
all-gcc
all-target-libgcc
all-target-libstdc++-v3
C++ hello：使用该级自己的 cc1plus 和 libstdc++
```

### 6.5 构建变量对账

下表描述本 pipeline 的实际边界。`CC_FOR_TARGET` / `GCC_FOR_TARGET` 在 `all-gcc` 阶段不参与目标代码生成；在 target 库阶段由 top-level Makefile 指向当前 stage 的 xgcc，而不是 host GCC。

| 阶段 | CC | CXX | CC_FOR_BUILD | CC_FOR_TARGET / GCC_FOR_TARGET |
| --- | --- | --- | --- | --- |
| 4.7 pass1 all-gcc | TCC | TCC | TCC | 不参与 |
| 4.7 pass1 libgcc | TCC | TCC | TCC | 当前 stage xgcc |
| 4.7 pass1 libstdc++ | TCC | pass1 `gcc/xgcc` | TCC | 当前 stage xgcc |
| 4.7 pass2 / 4.8+ | 前一级 wrapper gcc | 前一级 wrapper g++ | 前一级 wrapper gcc | 当前 stage xgcc |

这里的关键不是变量名字，而是控制权：

```text
生成目标代码的 cc1/cc1plus 必须来自当前或前一级自举链。
目标 `CC/CXX` 来自自举链；GCC 4.7 configure 自带的路径探测不会进入目标构建。
```

---

## 7. 验证与血统

### 7.1 已有证据

```text
C1 stage1 == stage2                逐字节一致
C1 原生 linker 重新链接             逐字节一致
C0 -> C1 产物                       与对照链一致
TCC self-host                       通过
TCC -> GCC 4.7 pass1/pass2          通过
GCC 4.7 -> 14.2                     all-gcc/libgcc/libstdc++/iostream
```

### 7.2 日志与构建树

```text
脚本：tests/gcc-ladder/run.sh
构建：/opt/gcc-ladder-final
日志：/tmp/gcc-ladder-final.log
提交：83b6f41
```

每次运行保留：

```text
$BUILD_BASE/log/<stage>-conf.log
$BUILD_BASE/log/<stage>-allgcc.log
$BUILD_BASE/log/<stage>-libgcc.log
$BUILD_BASE/log/<stage>-libstdcxx.log
$BUILD_BASE/log/<stage>-hello.out
```

### 7.3 尚未实现的强验证

以下措施尚未落地，不能写成已完成：

```text
在每个 stage 注入 tcc_bloodline_tag，并用 nm 审计 cc1
对每一级 GCC 做逐字节固定点比较
完整 GCC testsuite
完整 make install
```

当前血统验证依赖构建树路径、wrapper 指向、日志和源码补丁。它足以证明“目标库不是 host 直接生成”，但还不是最终的二进制级审计方案。

### 7.4 证明边界：工程验证与形式化证明

“seed -> C1 -> TCC -> GCC 14.2 全链跑通”是对**可自举性**的工程验证证明：

```text
seed/TCC 起始链可以重建 C1
C1 可以自举，stage1 == stage2
C1 可以构建 TCC，TCC 可以自举
GCC 4.7 两遍可以重建自身
GCC 4.7 可以逐级构建到 GCC 14.2
每级 all-gcc/libgcc/libstdc++/C++ iostream 可运行
```

它不能直接推出：

```text
所有输入下编译器都正确
所有 ABI 边界都已覆盖
构建完全可复现、无宿主污染
二进制没有后门
seed 信任根已经消失
```

形式化证明之所以困难，是因为证明对象不是一个函数，而是整条语义链：

```text
C/C++ 语言语义
优化器变换正确性
x86_64 SysV ABI
ELF/链接器/加载器/系统调用
libgcc/libstdc++ 运行时
seed 二进制和构建环境
```

GCC 14 体量的完整形式化验证不现实。CompCert 这类工作证明了 C 子集编译器的形式化是可行的，但它不是 GCC，也不覆盖 seed -> GCC 14 全链。

因此本项目的证明策略是分层的：

```text
工程证明：固定点、差分测试、ABI 测试、逐级构建、日志审计
可复现证明：seed 哈希、hex 物化、固定 build tree、确定性输入
形式化边界：只对 seed 解码器/A0/A1/C0/C1 核心语义做局部形式化
```

白皮书中应避免写“已证明编译器绝对正确”，应写“已验证该自举链在给定输入、构建环境和测试集合下可行且自洽”。

---

## 8. 复现

```sh
cd /home/quqiufeng/bit
JOBS=4 BUILD_BASE=/opt/gcc-ladder-final sh tests/gcc-ladder/run.sh
```

依赖：

```text
TCC:   build/tcc/bit-tcc
源码:  /opt/gcc-4.7.4 ... /opt/gcc-14.2.0
依赖:  /opt/gcc47-deps（MPFR/MPC）
系统:  as/ld、glibc、crt、系统头文件
```

`run.sh` 跳过已有 `<stage>-hello.out` 的阶段，因此可以在超时或中断后继续；使用全新的 `BUILD_BASE` 会从 4.7 pass1 重新开始。

---

## 9. 局限与路线图

### 9.1 局限

```text
完整 make install 未逐级通过
完整 GCC testsuite 未逐级通过
C1 -> TCC 尚未并入同一个 driver
seed.elf 仍是剩余信任根
自有 binutils/glibc/sysroot 未完成
```

### 9.2 路线图

```text
P1 把 C1 -> TCC 接入 seed -> GCC 14.2 的统一 driver
P2 清理/固定 configure 的宿主工具路径探测（非功能阻塞）
P3 完成逐级 make install 与 testsuite
P4 用 tcc_bloodline_tag + nm 建立二进制级血统审计
P5 构建自有 binutils/glibc/sysroot，缩小系统输入
P6 建立可复现构建矩阵与独立验证报告
P7 可选：对 seed/A0/A1/C0/C1 核心语义做局部形式化或半形式化验证；不把 GCC 14 全链作为形式化目标
```

最终目标不是宣称“没有任何外部依赖”，而是：

```text
把信任根缩小到可审计的 seed，
让 seed 之上的每一级都能被重新构建、逐字节比较和独立验证。
```

这既是编译器自举的技术问题，也是每个程序员迟早会面对的“第一推动力”问题。
