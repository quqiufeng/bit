# BOOT：从火种到 TCC 的原始时期

本文只讲一段：从仓库里唯一一个 5 KiB 的二进制火种，走到一个能编译 TCC、TCC 又能编译自己的 C1 编译器。

这一段之所以"没有文档"，是因为它不是一个编译器在自举，而是**六个不同层次的程序接力**，每一层的输入输出格式都不一样（hex 文本、字节映像、A1 汇编文本、C 源码、ELF 目标文件）。只看单个脚本（`native-c1.sh` 三十几行）看不出为什么要有这六层，也看不出每一层"什么时候算成功"。

后面的事（TCC 编译 gcc、编译正常程序）属于常规世界：有 libc、有动态链接器、有 `ar`，不需要本文。本文到此为止。

---

## 1. 全景

```text
                    ┌──────────────── 宿主 gcc/cc 只在这里出现（生成种子，产物已固化+哈希）
                    │
seed.S ──cc──▶ seed.elf (5088 B, 手写 hex 解码器)          ← 唯一提交的二进制根
                    │
                    ├─ seed.elf < seed.hex        == seed.elf        （自产自）
                    ▼
              A0 (10232 B)   ← seed.elf < a0.seed.hex
                    │  输入: hex 字节 + ^地址 + :偏移 + %相对引用
                    │  输出: ELF64 可执行映像
                    ├─ A0 < a0.seed.hex           == A0              （自产自）
                    ▼
              A1 (26376 B)   ← A0 < a1.seed.hex
                    │  输入: + 命名标签 :name %name "str" .zero .org
                    │  输出: 同一套 ELF 映像
                    ├─ A1 < a1.seed.hex           == A1              （自产自）
                    ▼
              C0v2 (32 MiB 定长映像)  ← A1 < c0v2.a1
                    │  输入: C 子集源码（stdin）
                    │  输出: **A1 汇编文本**（stdout），不是 ELF
                    ├─ C0v2 < c0v2.c              == c0v2.a1        （文本不动点）
                    ├─ A1 < 上面的 c0v2.a1        == C0v2           （二进制不动点）
                    ▼
              c1-bootstrap (32 MiB)   ← C0v2 < compiler-bootstrap.c → bootstrap.a1 → +helpers.a1 → A1
                    │  冻结的 C1 源码 + 抬高的分配器上限 + 内嵌 syscall shim
                    │  能编译"扩容后的 C1 源码"，但自己还编不动 TCC
                    ▼
              最终 C1 + c1l           ← c1-bootstrap < compiler.c/runtime.c/linker.c
                    │  c1l 由 C0v2 从 linux/standalone-*.c 构建（自带 ELF 链接器）
                    ├─ c1.elf 编 compiler.c → compiler.o
                    ├─ 链接 stage1 = runtime.o + compiler.o
                    ├─ stage1 编 compiler.c → compiler-stage2.o → stage2
                    ├─ cmp stage1 stage2                             （不动点）
                    ▼
              TCC 0.9.27              ← C1 < tcc.c (ONE_SOURCE 展开)
                    │  ★ 唯一一次需要宿主 gcc：链接 tcc.o + c1-init.o -ldl -lm
                    ▼
              bit-tcc-bin
                    ├─ 用它自己编 libtcc1.a（TCC 的运行时）
                    ├─ 用它自己编 tcc.c → tcc-self1.o
                    ├─ 再编一次       → tcc-self2.o，cmp 相同            （不动点）
                    └─ 链接 tcc-stage2，自举闭合 → 从这里开始进入"常规世界"
```

一句话版本：

> **seed 解字节 → A0 解地址 → A1 解标签 → C0 解 C 子集并退化成 A1 → c1-bootstrap 解完整 C → C1 解 TCC → TCC 解自己。**

每加一层，输入语言的抽象度升一级，输出仍然压在下一层能吃的格式上。这就是"原始时期"的全部结构。

---

## 2. 分层表（拿走即用）

| 层 | 产物 | 源码/种子 | 输入语言 | 输出 | 规模 |
|---|---|---|---|---|---|
| L0 | `seed.elf` | `linux/seed.S`，`seed.hex` | hex 字符流（支持 `;` 注释） | 原始字节 | 5088 B |
| L1 | A0 | `a0/a0.S` + `a0.seed.hex` | hex 字节、`^`、`:HHHH`、`%HHHH` | ELF64 可执行 | 10232 B |
| L2 | A1 | `a1/a1.c` + `a1.seed.hex` | A0 + 命名标签/字符串/`.zero`/`.org` | ELF64 可执行 | 26376 B |
| L3 | C0 / C0v2 | `c0/c0.c`、`c0/c0v2.c`、`c0v2.a1` | C 子集源码（stdin） | **A1 汇编文本** | 1697 行 → 134 KB A1 |
| L4 | c1-bootstrap | `native/compiler-bootstrap.c` | 同上 | 32 MiB 定长 ELF | 338 KB 源码 |
| L5 | C1 + c1l | `linux/compiler.c`、`runtime.c`、`linker.c`、`standalone-*.c` | 完整 C（含指针/char/结构体） | ELF64 `.o`（stdout） | 11989 行 / 372 KB |
| L6 | TCC | `tcc-0.9.27/` | 标准 C | ELF `.o` + 自建链接 | `tcc.o` 928 KB |

所有层都不链接 libc（L6 的第一次链接除外，见 §6）。所有层都是"stdin 读源、stdout 写产物"的过滤器形态，这是刻意的：**没有文件名、没有搜索路径、没有 `-o`，只有字节流**，因为文件系统操作是下一层才敢碰的东西。

---

## 3. 逐层

### L0 `seed.elf`：火种只做一件事

`bootstrap/linux/seed.S` 是全部信任的根：一个 ~80 行、只用 `read`/`write`/`exit` 三个系统调用的循环。读到两个 hex 字符就拼成一个字节写到 stdout，跳过空白和 `;` 到行尾。

为什么是 hex 解码器而不是一个二进制？**因为信任面要能被眼睛审计**。一个 hex 解码器的语义可以写在半张纸上（"每两个字符是一个字节"），任何人可以手工验证它没有后门。它是唯一提交进仓库的二进制（`seed.elf`），且 `seed.hex` 能让它重新产出自己：

```sh
bootstrap/linux/seed.elf < bootstrap/linux/seed.hex | cmp - bootstrap/linux/seed.elf
```

从这一步往后，**再没有任何一环需要 C 编译器**。

### L1 A0：解地址，不解指令

A0 常常被误会成"汇编器"，它不是。它**不编码 x86 指令**——字节已经在源文件里了（写成 `48 89 E5` 这种）。A0 只做三件簿记：

| 语法 | 含义 |
|---|---|
| `; ...` | 注释（seed 阶段已经处理） |
| `48 89 E5` | 原样吐出一个字节 |
| `^AAAAAAAA` | 设定当前虚拟地址（origin），不产出字节 |
| `:HHHH` | 设定当前输出偏移，不产出字节 |
| `%HHHH` | 吐 4 字节小端，值是"目标地址 − 该字段末尾"的 PC 相对位移 |

ELF 头和 program header 也是 A0 源文件里手写的字节——和上游把 PE 头写进 `a0.a0` 是同一个思路。

这样切分的好处：**汇编器没有编码表，所以没有"编码表 bug"**。指令字节的来源是上层（A1 的人写，或 C0 生成），A0 只负责地址算术，而地址算术可以用 30 行看懂。

`a0_ref.c` 是宿主 C 写的语义 oracle，只用于对照测试；`a0.S` 是真正进链的实现（syscall-only）。`a0/selfhost.sh` 验证 `A0 < a0.seed.hex == A0`。

### L2 A1：加标签，仍然是"字节 + 地址"

A1 把 A0 的"数字地址"升级成"名字"，并补上写汇编必需的四样东西：

- `:name` / `%name` — 命名标签与命名相对引用
- `"text"` — 字符串（自动补 NUL）
- `.zero N` — 零填充（只接受十进制常量）
- `.org N` — 同时设定文件偏移与虚拟地址（十进制）

A1 是这一串里**唯一有双向验证**的层：

```sh
A0 < a1.seed.hex  == A1     # 由上一层造出
A1 < a1.seed.hex  == A1     # 自己造出自己
```

注意 `a1/a1_self.c` + `a1-selfhost.sh` 走的是"源级自举"：用 C0v2 的 **BSS 变体**（`c0v2_bss.c`，见 §7）编译 `a1.c` 得到 `stage1.elf`，再让它编译同一份 A1 汇编文本得到 `stage2.elf`，两者二进制相同。也就是说 A1 有两个独立入口：一份是 `a1.seed.hex`（字节级），一份是 `a1_self.c`（源码级）。

### L3 C0 / C0v2：C 子集 → **A1 文本**

这是整条链上最重要的一个设计取舍：

> **C0 不生成 ELF 目标文件，它生成 A1 汇编文本。**

把重定位、ELF 头、符号表这些"格式细节"全部推迟到 A1（它只会算相对位移）和后面的 C1（它才真正写 `.o`）。代价是 C0 只能生成"整个程序镜像"，不能生成可重定位目标；收益是 C0 的代码生成器可以短到 1697 行还能自举。

C0 的能力边界（随 c0 → c0v2 演进）：

- 函数、参数、局部变量；`if/else`、`while`、`for`、`break/continue`
- 调用、递归、算术/比较/逻辑/位运算
- 全局 `int`、`int[]`；c0v2 增加 `char` 宽度、1D/2D 数组、`++/--`、复合赋值、简单 cast、指针类型 `&` `*` `p[i]` 与指针算术
- 固定布局：代码段与数据段用 `.org` 硬编码分隔，镜像最后 `.zero` 补到 **32 MiB**（`c0v2.c` 末尾 `emit_zero(0x2000000 - generated_bytes)`）

两个不动点，缺一不可：

```sh
C0v2 < c0v2.c      == c0v2.a1     # 文本不动点：能重放自己的 A1 源
A1   < c0v2.a1     == C0v2        # 二进制不动点：A1 能重放 C0v2
```

`c0v2.a1`（134 KB）本身是提交在仓库里的**文本种子**，所以 C0v2 不需要任何二进制参与就能重建自己——这是整条链上"火种"真正交接的地方：从 `c0v2.a1` 起，后面全是文本。

### L4 `c1-bootstrap`：为什么要多一跳

到这里可以直接让 C0v2 去编译最终的 `linux/compiler.c` 吗？不行。原因不是语言能力，而是**容量**：

- `compiler.c` 已经长到 372 KB / 11989 行，内部用固定数组（源码 1 MiB、字符串池 1 MiB、标识符表 2048×256B……）
- C0v2 是"编译整个镜像"的编译器，能吃的源规模被自己的固定布局卡住
- 而旧 C1 的固定容量又装不下扩容后的 `compiler.c`

于是插一层桥：`native/compiler-bootstrap.c`（338 KB）是**冻结的 C1 源码，只把分配器上限抬上去**，额外把 syscall shim 内嵌进同一份源码（`compiler-runtime.c` = compiler + runtime 的合并校验版）。C0v2 编得动它；它编得动最终的 `compiler.c`。

```sh
C0v2 < compiler-bootstrap.c        > bootstrap.a1
insert-a1 + helpers.a1             → bootstrap-full.a1   # 补上 syscall/IO 桩
A1   < bootstrap-full.a1           → c1-bootstrap.elf
```

一份源码要能跑，还缺 `_start`、`read`/`write`、`mmap` 这些桩。C1 自己不支持内联汇编，所以这些用 `helpers.a1`（A1 写的 syscall 桩）拼进镜像——**桩也是文本，可审计**。

### L5 最终 C1 与 c1l：第一次出现真正的 ELF 目标文件

`c1-bootstrap` 用自己编译 `linux/compiler.c` 和 `linux/runtime.c`，输出**可重定位 ELF `.o`**（stdout）。谁把它们链接成可执行文件？**自己链**：`c1l.elf` 是 C0v2/A1 从 `linux/standalone-runtime.c` + `linux/standalone-link.c`（913 行，自带 ELF 读写、符号解析、重定位）构建的独立链接器。

链接器的接口极其朴素——从 stdin 读一份清单：

```text
<输出路径>
<入口符号>
<对象文件 1>
<对象文件 2>
...
```

于是：

```sh
c1-bootstrap < compiler.c > compiler.o
c1-bootstrap < runtime.c  > runtime.o
printf '%s\n' c1.elf _start runtime.o compiler.o | c1l.elf
```

C1 的验收由四条独立断言组成，不是"跑通了就算"：

1. `tiny.o` 开头是 `7f 45 4c 46`（真的产出了 ELF）
2. `compiler.o == compiler-i.o`——先 `-E` 展开再编，结果必须与直编一致（预处理/输出自洽）
3. `native-linker == native-linker2`——链接器能重新链接自己，字节相同
4. `stage1 == stage2`——C1 编译自己的源码，链接成 stage1；stage1 再编一次，链接成 stage2；两者字节相同

第 4 条就是标准的"自举不动点"，也是从这里开始，编译器进入可自我维护的状态。

关于源码形态的一个坑：`compiler.c` 里到处是 `__c1_linux_GetStdHandle`、`HeapAlloc`、`ReadFile` 这类 Win32 风味的名字。它们不是历史遗留垃圾，而是**稳定 ABI 层**：`runtime.c` 用 `mmap`/`read`/`write` 实现这些名字。这套接口是从上游 Win64 代码生成器迁到 SysV/Linux 时留下的"接缝"，好处是编译器主体一行不用改就能换宿主。读这段代码时记住：看到 Win32 名字，去 `runtime.c` 找实现。

### L6 TCC：第一次需要宿主链接器，然后就再也不需要了

C1 是 freestanding 的：它会写 `.o`，但它**不会动态链接**，而 TCC 必须链 glibc（`malloc`/`printf`/`dlopen`）。所以这里破一次例：

```sh
C1_INCLUDE="tcc-0.9.27;include"  c1 < tcc.c  > tcc.o
gcc -o bit-tcc-bin tcc.o c1-init.o -ldl -lm      # ★ 原始时期唯一一次宿主 gcc 链接
```

`tools/c1-init.c` 只有一个构造函数：`__attribute__((constructor)) c1_init_ctor(){ __c1_init(); }`——把 C1 自己的初始化钩子接到 glibc 的启动流程上。gcc 在这里**只编译这 3 行胶水并做链接**，TCC 的 1 万行源码全部是 C1 编出来的。

TCC 一旦跑起来，就自己接管：

```sh
bit-tcc -c lib/libtcc1.c -o runtime/libtcc1.o   # 自建运行时
ar rcs runtime/libtcc1.a ...
bit-tcc -B runtime -nostdinc -c tcc.c -o tcc-self1.o   # 自己编自己
bit-tcc -B runtime -nostdinc -c tcc.c -o tcc-self2.o
cmp tcc-self1.o tcc-self2.o                     # 一代不动点（同二进制的两次运行）
bit-tcc -B runtime -o tcc-stage2 tcc-self1.o -ldl -lm  # 自己链自己

# 真正的闭合要换一代编译器再编一次
tcc-stage2 -B runtime -nostdinc -c tcc.c -o tcc-self3.o
tcc-stage2 -B runtime -o tcc-stage3 tcc-self3.o -ldl -lm
tcc-stage3 -B runtime -nostdinc -c tcc.c -o tcc-self4.o
cmp tcc-self3.o tcc-self4.o                     # ★ 二代不动点
cmp tcc-stage2 tcc-stage3                        # ★ 连二进制也相同
```

实测：`tcc-self1.o == tcc-self2.o`（各 520256 B），`tcc-stage2 == tcc-stage3`（各 317276 B，sha256 `9f7c9762…`），说明 TCC 的代码生成对自身源码稳定。两代之间不要比：`tcc-self3.o`（TCC 编，520236 B）与 `tcc-self1.o`（C1 编）**本来就不同**，那是"两个编译器的产物"而非不动点。细节见 §5.2、§5.3。

`-B runtime` 不是可选项：没有它，`tcc-stage2` 会去找 `$PATH` 里的 gcc 头文件，撞上 `stdarg.h` 的 `__gnuc_va_list`。`upstream.sh` 生成的 `bit-tcc` 包装脚本已内置，裸二进制要自己加。

从 `bit-tcc-bin` 起，libc 调用、`-shared`、`-L/-l`、`ar` 都能用了——**原始时期结束**。

---

## 4. 不动点清单（一条命令一层）

这是本文最有用的部分：每一层都有"我怎么知道它没坏"的单一断言。

```sh
# L0 火种自产自
bootstrap/linux/seed.elf < bootstrap/linux/seed.hex | cmp - bootstrap/linux/seed.elf

# L0→L1→L2 整条字节级链（含 A0/A1 自产自）
bootstrap/linux/seed-selfhost.sh

# L3 C0v2 文本不动点 + 二进制不动点
bootstrap/linux/c0/c0v2-from-a1-selfhost.sh

# L2 源级不动点（A1 编 A1）
bootstrap/linux/a1/a1-selfhost.sh

# L3→L5 全链：种子里程碑 + C1 stage1==stage2 + TCC
bootstrap/linux/c0/native-c1.sh     # 或 ./build.sh / ./build-zero.sh

# L5→L6 TCC 自举（C1 编 TCC + TCC 自编自链）
C1=$PWD/build/fromzero/native-c1/c1.elf OUT=$PWD/build/fromzero/tcc tests/tcc/upstream.sh

# 过渡种子包（另一条入口，见 §8）
bootstrap/from-seed.sh
```

全部通过时，"唯一的二进制根是 `seed.elf`"这句话才是真的：`bootstrap/linux/c0/` 下只有 `seed.elf` 一个提交的二进制，其余 A0/A1/C1 全部按需生成。

---

## 5. 完整跑一遍：空目录 → TCC（实测记录）

§3 讲的是"为什么要有这些层"，这里讲"怎么一次跑完"。**两条脚本、约 12 秒、中间零人工干预**：

```sh
git clone git@github.com:quqiufeng/bit.git && cd bit

# ① 种子 → A0 → A1 → C0v2 → c1-bootstrap → C1 + c1l
BUILD=$PWD/build/fromzero ./bootstrap/native-zero.sh
#   PASS: native Linux C1 and native linker reach stage1 == stage2

# ② C1 编 TCC → TCC 自建运行时 → TCC 编自己
C1=$PWD/build/fromzero/native-c1/c1.elf \
OUT=$PWD/build/fromzero/tcc ./tests/tcc/upstream.sh
#   PASS: C1 -> TCC -> TCC self-host + TCC native link
```

`BUILD` / `C1` / `OUT` 三个变量是为了把产物落到一个干净目录（默认 `build/`，已在 `.gitignore` 里）。不设也行。两条脚本都是 `set -eu`，任何一层的 `cmp` 不过就立刻退出——**`PASS` 本身就是断言通过的回执**，不是"跑完了"的客套话。

### 5.1 实测产物（Ubuntu 24.04 / gcc 13.3 / 6 核）

| 层 | 产物 | 大小 | sha256[:16] | 谁编的 |
|---|---|---|---|---|
| L0 | `seed.elf` | 5088 B | `65788853cae62f45` | cc（产物已提交） |
| L1 | `a0.seed.elf` | 10232 B | `cc2fd3b2559aba8d` | `seed.elf < a0.seed.hex` |
| L2 | `a1.seed.elf` | 26376 B | `fb8e12b0321271ec` | `A0 < a1.seed.hex` |
| L3 | `c0v2.a1` | 134446 B | `7fedbb542fc080c2` | 仓库里固化的 A1 文本 |
| L4 | `c0v2-seed.elf` | 32 MiB | `636a7c94c74c6837` | `A1 < c0v2.a1` |
| L4 | `c1-bootstrap.elf` | 32 MiB | `efadc4d022e94e3d` | `C0v2 < compiler-bootstrap.c` + `helpers.a1` |
| L5 | `c1.elf` = `stage1` | 631440 B | `3eee9994795cf3ad` | `c1-bootstrap < compiler.c` + `c1l` 链接 |
| L5 | `c1l.elf`（32 MiB 镜像） | → `native-linker` 83760 B | `d1606d6b…` / `c70f84e1…` | `C0v2 < standalone-*.c` |
| L6 | `tcc.o` | 928824 B | `5b64414b1dcf41c4` | `C1 < tcc.c` |
| L6 | `bit-tcc-bin` | 744864 B | `2ca814bd8832ed84` | **gcc 只负责链接** |
| L6 | `tcc-self1.o` = `tcc-self2.o` | 520256 B | `9dc6ff9819305c33` | `bit-tcc < tcc.c` |
| L6 | `tcc-stage2` = `tcc-stage3` | 317276 B | `9f7c9762ad5bf467` | TCC 链自己 |

哈希全部是**跨运行可复现**的：换一个 `BUILD` 目录重跑，同一层的产物逐字节相同。这是"链没坏"的第二个证据，比单次 `cmp` 更强——它同时排除了"两次都错得一样"以外的偶然。

### 5.2 TCC 自举要怎么算闭合

单次 `cmp tcc-self1.o tcc-self2.o` 只是**同一份二进制跑到两次结果一样**，说服力有限。真正的判据是**换一代编译器再编一次**：

```sh
bit-tcc    -nostdinc -c tcc.c -o tcc-self1.o   … 编出 self1
bit-tcc    -o tcc-stage2 tcc-self1.o -ldl -lm  … 第一代 TCC
tcc-stage2 -nostdinc -c tcc.c -o tcc-self3.o   … 第二代产物
tcc-stage2 -o tcc-stage3 tcc-self3.o -ldl -lm  … 第二代 TCC
tcc-stage3 -nostdinc -c tcc.c -o tcc-self4.o
cmp tcc-self3.o tcc-self4.o   # ← 这才是闭合
cmp tcc-stage2  tcc-stage3    # ← 而且二进制也相同
```

两条 `cmp` 都过，才是"TCC 能编译 TCC"。`tcc-stage3 -v` 输出 `tcc version 0.9.27 (x86_64 Linux)`。

注意 `tests/tcc/upstream.sh` 只跑到 `tcc-stage2`（一代不动点 + 原生链接），二代那三行目前是手工补的——所以 §4 清单里的 `PASS` 是**必要条件**，§5.2 的两条 `cmp` 才是充分条件。

### 5.3 三个实测踩到的坑

1. **跨编译器的 `cmp` 没有意义。** `tcc-self1.o`（C1 编）与 `tcc-self3.o`（TCC 编）**不同**，从第 41 字节起分叉。这不是链断了，是两个编译器生成的机器码本来就不同。不动点只能在**同一编译器代际内**比较（self3 == self4），跨代比的是产物，不是不动点。
2. **`tcc-stage2` 必须带 `-B runtime`。** 不带的话它会退回 `$PATH` 里的 gcc，把 `/usr/lib/gcc/.../include/stdarg.h` 拉进来，报 `';' expected (got "__gnuc_va_list")`。TCC 自举靠的是自建的 `runtime/include`，不是宿主的头文件。`upstream.sh` 生成的 `bit-tcc` 包装脚本已经内置了这个 `-B`，直接调裸二进制时要自己加。
3. **`-nostdinc` 不能省。** 自举命令带 `-nostdinc -Itcc-0.9.27/include -Iinclude`，最后那个 `include/` 是本仓库的兼容层（补 `__REDIRECT` 一类 glibc 宏）。少了它同样会撞上宿主头文件。

三条坑的共同含义：**TCC 自举的边界不在"能不能编自己"，而在"它吃哪套头文件"。** 见 §9 第 4 条。

### 5.4 跑完之后怎么瘦：只留可执行件

阶梯跑完是 19 GB（`.o` 6.1 GB / `.a` 2.3 GB / 二进制 5.2 GB / 其余是头文件与缓存），其中 14.2 的 `cc1` 单个 324.8 MB，strip 后 38 MB。跑完就不必再留构建树，`tests/gcc-ladder/prune.sh` 把它压成"能编 C 和 C++"的最小集合：

```sh
# 每级: /opt/gcc-ladder-2026/<stage> -> /opt/gcc-ladder-bin/<stage>
sh tests/gcc-ladder/prune.sh /opt/gcc-ladder-2026 /opt/gcc-ladder-bin
```

保留的东西：`cc1`/`cc1plus`/`lto1`/`lto-dump`/`xgcc`/`collect2`/`lto-wrapper`/`liblto_plugin.so`、`crt*.o`、`libgcc*.a`、`libgcov.a`、`libgcc_s.so`、`libstdc++.so`、`include/` 与 `include/c++/<ver>/`；其余（`.o`、`.a` 中间件、`.gch` 预编译头）全部丢弃。全部 `strip`，每级写一个 `smoke-c`/`smoke-cxx` 包装脚本（`-Wl,-rpath,$D/lib -lstdc++`）现场编一个 C 和一个 C++ 程序并运行，通过才落盘。

实测：10 级合计 **1.3 GB**（58–196 MB/级），干净环境（`env -i`）逐级跑 `smoke-c`/`smoke-cxx` 全绿，`MANIFEST.txt` 给每个文件的内容哈希。

三个坑：

1. **头文件是符号链接，指向源码树。** 源码树按 §5.1 跑完即删，链接就悬空了。`prune.sh` 从 tarball 里把真身取出来（`.hdr` 缓存 + `cpio`），再在 `include/c++/<ver>/` 下重建 `std/*`、`bits/*` 这些目录级链接（14.2 一级 816 个）。
2. **老版本的 `gcc/liblto_plugin.so` 也是链接**（指向 `lto-plugin/.libs/`）。复制时必须 dereference，否则新树里是个断路链接，`xgcc` 一用就报 `-fuse-linker-plugin, but liblto_plugin.so not found`——这正是 4.7–9.5 头一轮全 FAIL 的原因。
3. **`libstdc++.so` 不在 `lib/` 就链接失败**（`std::cout` undefined），包装脚本要把 `-lstdc++` 和 `-rpath` 一起给。

裁剪后构建树可以直接删（`log/`、`wrap/`、`driver.log` 与 `MANIFEST.txt` 是审计证据，留着）：

```sh
rm -rf /opt/gcc-ladder-2026/{4.7-pass1,4.7-pass2,4.8,4.9,5.5,7.5,9.5,12.3,13.3,14.2}
# 18.5 GB -> 18 MB（只剩日志），可执行件在 /opt/gcc-ladder-bin，1.3 GB
```

---

## 6. 宿主工具出现在哪里（信任边界）

原始时期里，宿主 C 编译器/工具只出现在下面这些位置，而且**产物都进哈希清单**（`bootstrap/linux/c0/native/seeds.sha256`、`bootstrap/seed.sha256`）：

| 位置 | 用途 | 是否在推理路径上 |
|---|---|---|
| `seed.S` → `seed.elf` | 生成火种（cc） | 否，一次性，产物已提交 |
| `a0/make-seed.sh` | `a0.S` → `a0.seed.hex`（cc） | 否 |
| `a1` 种子生成 | `a1.c` → `a1.seed.hex` | 否 |
| `tests/tcc/upstream.sh` | `tcc.o` + `c1-init.o` → `bit-tcc-bin`（gcc 链接） | 是，但只链接不编译 |

除此之外，`native-c1.sh` 全程不调用 gcc、不调用系统 `ld`、不调用 Python。历史上曾经存在的 PE 桥（`bootstrap/minimal-chain.sh` + `tools/pe2elf.c`）已经退为旁支：它证明了上游 PE 链能在 Linux 上跑，但产物是上游 Windows C1，不是现在这条 SysV 链。

---

## 7. 容量与硬约束（为什么代码长成这样）

读源码时会反复撞到这些数字，它们不是随手写的：

| 约束 | 值 | 含义 |
|---|---|---|
| C1 单文件源上限 | 1 MiB（`source[1048576]`） | 超了直接报错退出 |
| C1 字符串池 | 1 MiB | 同上 |
| 标识符表 | 2048 × 256 B | 超了丢名字 |
| C0v2 输出镜像 | 定长 32 MiB（`.zero` 补齐） | 布局固定才能"整镜像编译" |
| `.zero` / `.org` 参数 | 仅十进制常量 | A1 不解析表达式 |
| 帧布局 | 标量 8 字节槽、数组按元素宽、16 字节对齐 | 避免 char 紧凑槽的复杂度 |
| 参数落位 | 前 6 个 SysV 参数溢出到 `[rbp-8]`,`[rbp-16]`… | 见 `c0/README.md` |

`c0v2_bss.c` 的存在也是容量问题：A1 的源码自举要编译 `a1_self.c`，它需要比默认布局更大的 BSS，于是有一个只改段布局的 C0v2 变体（`c0v2-bss-build.sh`）。这类"变体"是原始时期的典型产物——**能力不够时改布局，而不是改语言**。

---

## 8. 两条入口（别被目录结构迷惑）

仓库里目前有两条都能走到 C1 的路，别混：

**A. 上游种子链（主路径，本文主角）**

```text
seed.elf → A0 → A1 → C0v2 → c1-bootstrap → C1 + c1l → TCC
```
入口：`bootstrap/linux/c0/native-c1.sh`（被 `build-zero.sh` → `build-core.sh` → `build.sh` 调用）

**B. 过渡种子包（历史入口，仍可用）**

```text
bootstrap/seed-c1.hex + seed-link.hex + seed-startup.hex + seed-syscalls.hex
  → from-seed.sh → seed-stage1 → seed-stage2（C1 不动点）→ hello
```
入口：`bootstrap/from-seed.sh`

B 是"先把 C1 跑起来"的产物，A 是"从字节火种长出来"的产物。B 里的四个 hex 是预先固化的二进制，A 里只有 `seed.elf` 一个。理解原始时期要看 A；做快速回归可以跑 B。

---

## 9. 还没闭合的部分

诚实记录，避免下一个人以为已经做完了：

1. **`seed.elf` 本身仍是 cc 编译 `seed.S` 得来的**。`seed.hex` 让它自产自，但"第一个 seed 怎么来的"仍依赖宿主汇编器。目标是一个手写的极小 seed 解码器（`a0/README.md` 末尾记着这个 gap）。
2. **A1 的源级自举依赖 C0v2**（`a1_self.c` 路径），还不是"A1 从 A0 长出来"的纯字节叙事。
3. **C1 不支持内联汇编**，`_start`/syscall 桩靠 `helpers.a1` 和 `c1-init.c` 补，属于"文本桩 + 一次宿主链接"的混合状态。
4. **TCC 的第一次链接需要 gcc/glibc**。彻底闭合需要 C1 支持动态链接输出（或一个自写的动态链接器）。

这四点的共同点是：它们都还不是"从种子到 TCC 全文本"。但链的**推理路径**已经完整：每一层的产物都能用更下一层重放，且每一层都有字节级断言。

---

## 10. 一页速查

```text
seed.elf  五 KiB。hex → 字节。唯一的二进制根。
A0        字节 + ^地址 + %相对引用 → ELF。不编码指令。
A1        + 标签/字符串/.zero/.org → ELF。A0 和 A1 都能造它。
C0v2      C 子集 → A1 文本。定长 32 MiB 镜像。自己重放自己。
c1-boot   冻结 C1（抬容量）→ 全链桥。C0v2 编它，它编最终 C1。
C1 + c1l  完整 C → ELF .o，自带链接器。stage1 == stage2。
TCC       C1 编 TCC（gcc 只做一次链接），TCC 自建运行时并编自己。

两种 cmp：
  一阶   同一个二进制跑两次结果相同（seed/A0/A1/C0v2/C1 都靠它）
  二阶   换一代编译器再编一次，产物与二进制都相同（只有 TCC 做到）
```

时间与哈希（§5）：空目录到 TCC 约 12 秒（L0–L5 4.3 s，L5–L6 7.9 s），所有产物跨运行逐字节可复现。

一句话：**每一层只做"下一层需要的那一件事"，并且每一层都有一个 `cmp` 能证明它没坏。** 原始时期的全部复杂度，就花在这两件事上。
