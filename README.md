# BitKindle Native

> **从一个 5088 字节的手写火种出发，一层层长出一个能编译 TCC、TCC 又能编译自己的 C 编译器，
> 再经 GCC 4.7 两遍自举爬上天梯，直到 GCC 14.2。**
>
> 整条链只用两条脚本、约 12 秒，中间零人工干预；每一层都有一条 `cmp` 断言，
> 每一件产物跨运行逐字节可复现。

本项目解决的是**编译信任链**问题：你机器上的编译器是二进制，那个二进制由更早的编译器编出来，
一路追溯到一个谁也审计不了的起点。这里把那根链的"根"压缩到 5 KiB 可以眼睛读完的机器码，
并让链上每一步都能被重新执行、被逐字节验证。

本项目受 [tombkeeper/BitKindle](https://github.com/tombkeeper/BitKindle) 启发，把"从零构建自己的 C 工具链"
带到 Linux，同时保留 Windows 的原生实现。**不是跨平台编译器，不做翻译层，不追求二进制或目标文件互通。**

---

## 1. 为什么要有这条链

现代软件世界的底层是一个黑箱：`gcc` / `clang` 是几千万行的程序，发行版只给你编译好的二进制，
它由上一代二进制编出来，代代相传。你可以读源码，但**你无法证明你跑的那个二进制就是那份源码的产物**。
这条链追到某个点就断了，"信任"在那里变成信仰。

这个项目做的是把断点尽可能往小推：

- 仓库里唯一提交的二进制根是 `seed.elf`（5088 B）——一个 ~80 行、只用 `read`/`write`/`exit`
  三个系统调用的 hex 解码器。语义能写在半张纸上，可以手工审计它没有后门。
- 它自产自（`seed.elf < seed.hex == seed.elf`），之后一切——A0、A1、C0、C1、TCC、GCC——**全部由链上产物按需生成**，
  没有第二个提交的二进制。
- 每加一层，输入语言的抽象度升一级，输出仍压在下一层能吃的格式上。
  这是"原始时期"的全部结构，细节见 [docs/boot.md](docs/boot.md)。

一句话版本：

> **seed 解字节 → A0 解地址 → A1 解标签 → C0v2 解 C 子集 → c1-bootstrap 解完整 C → C1 解 TCC → TCC 解自己。**

## 2. 链长什么样

```text
seed.elf (5 KiB, 唯一的二进制根)
  ├─ 自产自:  seed.elf < seed.hex == seed.elf
  ▼
A0 (10 KiB)        hex 字节 + ^地址 + %相对引用        → ELF64 映像，不编码指令
  ▼
A1 (26 KiB)        + :name %name "str" .zero .org      → 同一个 ELF 映像
  ▼
C0v2 (32 MiB 镜像) C 子集源码                          → A1 汇编文本（定长，可 cmp）
  ▼
c1-bootstrap       C0v2 编一份抬了容量的冻结 C1        → 能编"完整 C"，是通往最终 C1 的桥
  ▼
C1 + c1l           完整 C（指针/char/结构体/全局）      → ELF .o / 自带链接器
  │  stage1 == stage2（C1 编译自己，两代产物逐字节相同）
  ▼
TCC 0.9.27         C1 编 1 万行 TCC；gcc 只做一次链接   → 原始时期结束
  │  TCC 自建 libtcc1.a → TCC 编自己 → 换一代再编一次仍是同一份二进制
  ▼
GCC 4.7（两遍） → 4.8 → 4.9 → 5.5 → 7.5 → 9.5 → 12.3 → 13.3 → GCC 14.2
```

| 层 | 产物 | 输入 | 断言（"我怎么知道它没坏"） |
|---|---|---|---|
| L0 | `seed.elf` 5088 B | `seed.hex` 字符流 | 自产自逐字节相同 |
| L1 | A0 10232 B | hex 字节 / `^` / `%` | `A0 < a0.seed.hex == A0` |
| L2 | A1 26376 B | A0 语法 + 标签/字符串 | `A1 < a1.seed.hex == A1` |
| L3 | C0v2 134446 B A1 文本 | C 子集源码 | 文本不动点 + 二进制不动点 |
| L4 | c1-bootstrap 32 MiB | 同上 | 能编出最终 C1，且该 C1 自举闭合 |
| L5 | `c1.elf` 631440 B + `native-linker` | 完整 C | `stage1 == stage2` |
| L6 | `tcc.o` 928 KB → `bit-tcc-bin` | 标准 C | 二代不动点：`self3 == self4`、`stage2 == stage3` |
| 天梯 | 10 级 GCC | TCC → 4.7 ×2 → 14.2 | 每级 all-gcc/libgcc/libstdc++ + C++ iostream 实跑 |

天梯里的每一级都是**真编译器**：它编自己、编 C++ 标准库、编一个 hello 并运行。
4.7 之所以要两遍，是因为它第一次只能用 TCC 的 C 编译器，第二遍才用上自己的 C++ 前端，
后面 4.8 起全部由上一级的 C++ 能力承接。

## 3. 它证明了什么，没证明什么

这是本文档最想讲清楚的一点，也是最容易被误读的一点。

**它证明的是**：存在一条从 5 KiB 可审计机器码到发行版级 GCC 14.2 的完整、可重放、逐字节可验证的构建路径。
每一步的产物要么由更下一层重放得到，要么有跨运行一致的哈希。这是**一份工程证书（certificate）**：
它把"我用了一个黑箱编译器"变成"我跑了一条我能从头检查的链"。

**它没有证明**：链上没有任何后门。不动点只证明**一致性**——编译器编自己得到同一份输出，
不排除"两代都错得一样"（Thompson 的 `login` 后门正是这样传播的）。真正的对手模型需要的是
可复现构建的**多方独立验证**，而这里的价值在于：参与验证的那一方，现在只需要审计 5 KiB。

**信任根仍然存在**，写在这里而不是藏在脚注：

- `seed.elf` 由宿主 `cc` 汇编 `seed.S` 得到（产物已提交并哈希，不在推理路径上）。
- 用户程序仍然用系统 `ld`/glibc；C1 自身 freestanding，但 TCC 的**第一次**链接需要一次 `gcc`（只链接，不编译）。
- C1 不支持内联汇编，`_start`/syscall 桩靠 `helpers.a1` + 一次宿主链接补齐。

这些是"还没闭合"，不是"假装闭合"。四个未闭合项和取舍理由逐条列在 [boot.md §9](docs/boot.md)。

## 4. 实测证据（Ubuntu 24.04 / gcc 13.3 / 6 核）

**种子 → TCC，两条命令，12196 ms**

```sh
BUILD=$PWD/build/fromzero ./bootstrap/native-zero.sh
#   PASS: native Linux C1 and native linker reach stage1 == stage2
C1=$PWD/build/fromzero/native-c1/c1.elf OUT=$PWD/build/fromzero/tcc ./tests/tcc/upstream.sh
#   PASS: C1 -> TCC -> TCC self-host + TCC native link
```

耗时拆分：L0–L5 4273 ms，L5–L6 7923 ms。换一个空目录重跑，16 个共享产物 sha256 全同。

| 层 | 产物 | 大小 | sha256[:16] | 谁编的 |
|---|---|---|---|---|
| L0 | `seed.elf` | 5088 B | `65788853cae62f45` | cc（唯一提交的二进制） |
| L1 | `a0.seed.elf` | 10232 B | `cc2fd3b2559aba8d` | `seed.elf` |
| L2 | `a1.seed.elf` | 26376 B | `fb8e12b0321271ec` | A0 |
| L3 | `c0v2.a1` | 134446 B | `7fedbb542fc080c2` | 固化的 A1 文本 |
| L5 | `c1.elf` = stage1 | 631440 B | `3eee9994795cf3ad` | c1-bootstrap + c1l |
| L6 | `tcc.o` | 928824 B | `5b64414b1dcf41c4` | C1 |
| L6 | `tcc-stage2` = `tcc-stage3` | 317276 B | `9f7c9762ad5bf467` | TCC 链自己 |

**TCC → GCC 14.2，一次干净跑通**

```sh
JOBS=6 SRC_BASE=/opt/src/gcc BUILD_BASE=/opt/gcc-ladder-2026 sh tests/gcc-ladder/run.sh
#   PASS: 4.7-pass1 … 14.2        （10 级）
#   PASS: TCC -> GCC 4.7 (2 passes) -> GCC 14.2
```

每个阶段留下 `log/<name>-witness.txt`（编译器版本、自编 C/C++ 程序的运行结果与哈希）和
`log/<name>-MANIFEST`（cc1/cc1plus/xgcc/libstdc++ 的大小与哈希）——审计证据先落盘，再谈删构建树。

**跑完怎么瘦**：19 GB 构建树 → 1.3 GB 可执行件。`tests/gcc-ladder/prune.sh` 每级只保留
`cc1`/`cc1plus`/`lto1`/`xgcc`/`collect2`/`lto-wrapper`/`liblto_plugin.so`、`crt*.o`、`libgcc*`、
`libstdc++.so` 和头文件，全 `strip`，并生成 `smoke-c`/`smoke-cxx` 包装脚本现场验证：

```sh
sh tests/gcc-ladder/prune.sh /opt/gcc-ladder-2026 /opt/gcc-ladder-bin
#   10 级合计 1.3 GB（58–196 MB/级），env -i 下逐级 C 与 C++ 全绿
#   4.7-pass1: c-ok 4.7.4     14.2: cxx-ok 14.2.0
```

## 5. 这条链能拿来干什么

1. **可复现构建的独立验算路径。** 想验证你手上的 gcc 有没有被动过手脚，不必从零写编译器：
   从 5 KiB 种子起跑一遍，比对你构建出的产物与发行版是否一致。审计成本从"几千万行"降到"5088 字节"。
2. **一个能长期自持的工具链，尺寸可控。** 裁剪后每级 60–200 MB，能编 C 和 C++、能编自己的标准库，
   不依赖外部 C 编译器（构建 C1 不需要 gcc，只有 TCC 首次链接破例一次）。
3. **接入现有 Linux 生态。** 14.2 就是发行版级编译器：libc/POSIX、`pthread`、socket、
   epoll/inotify/eventfd、`dlopen`、BusyBox 1.36.1 defconfig、cJSON、SQLite、Lua 5.4.7、
   QEMU 最小内核 + SSH 都已在本链产物上跑过。
4. **一个能读完的教学与审计样本。** 每层 80 行到 1.2 万行，输入输出都是 stdin/stdout 过滤器形态：
   没有文件名、没有搜索路径、没有 `-o`，只有字节流。想搞清"编译器是从哪儿来的"，这条链可以一节一节读。

## 6. 边界

- **不是跨平台编译器。** Linux x86-64（SysV/ELF64/glibc）与 Windows x86-64（MS x64/PE32+/自带 CRT）
  是两条独立线，只共享语言前端与纯算法；不共享 ABI、运行时、目标文件格式、链接器。
  不做 Wine/WSL/MinGW 那一套"兼容层"，也不承诺两边二进制互通。
- **不做 C++ 编译器。** 不做 C++ ABI/STL/异常。天梯里的 C++ 能力来自真正的 GCC，
  不是 C1 自己实现的。
- **不做形式化证明。** 这条链是证书不是定理；不动点证明一致性，不证明无害。
- **Windows 侧**沿用上游冻结参考实现，尚未整合进本仓库构建。

## 7. 快速开始

```bash
git clone git@github.com:quqiufeng/bit.git && cd bit

# ① 零基自举：seed → A0 → A1 → C0v2 → C1 + c1l（不需要 gcc/ld）
BUILD=$PWD/build/fromzero ./bootstrap/native-zero.sh

# ② C1 编 TCC，TCC 自建运行时并编自己
C1=$PWD/build/fromzero/native-c1/c1.elf OUT=$PWD/build/fromzero/tcc ./tests/tcc/upstream.sh

# ③ 上天梯（需事先下载 9 个 GCC tarball：tests/gcc-ladder/fetch.sh）
JOBS=6 SRC_BASE=/opt/src/gcc BUILD_BASE=/opt/gcc-ladder sh tests/gcc-ladder/run.sh
```

日常开发用 `./build.sh`（gcc stage0 路线，最快）：它产出 `build/c1-stage0/1/2` 并断言
`compiler1.o == compiler2.o`、`runtime1.o == runtime2.o`、`stage1 == stage2`、hello 可跑，
再跑 `tests/sysv`、`tests/hosted`、`tests/interop`（C1 ↔ gcc 双向 ABI）、`tests/reject`。

## 8. 入口速查

| 命令 | 作用 |
|---|---|
| `bootstrap/linux/c0/native-c1.sh` | **主入口**：零基种子 → C1 + c1l（由 `native-zero.sh` 调用） |
| `bootstrap/from-seed.sh` | 历史过渡入口：固化的 4 个 hex → C1 不动点（快速回归用） |
| `tests/tcc/upstream.sh` | C1 → TCC → TCC 自举 + TCC 原生链接 |
| `tests/gcc-ladder/run.sh` | TCC → GCC 4.7 两遍 → 14.2，逐级 witness |
| `tests/gcc-ladder/prune.sh` | 构建树 → 可执行件（19 GB → 1.3 GB） |
| `tests/selfhost-pipeline.sh` | 两阶段全景：C1 → TCC → Lua/BusyBox；TCC → GCC 14.2 |
| `tests/mini-kernel/ssh.sh` | QEMU 最小内核 + BusyBox + Lua + SSH |

## 9. 文档

- **[docs/boot.md](docs/boot.md)** — 从火种到 TCC 的原始时期：逐层讲清每一层为什么存在、
  不动点清单、实测哈希与耗时、三个实测踩到的坑、信任边界、未闭合项、一页速查。
- `tests/gcc-ladder/SOURCES.md` — 源码策略：只留 9 个 tarball + SHA256SUMS，解压树按需重建。
- 各条入口的用法见所在目录的 `README.md`。
- 历史设计文档（`DESIGN.md`、`WHITEPAPER.zh-CN.md`、`C0_REFACTOR.md`、`SEED_GRAPH.md` 等）
  已于 2026-11 清理，需要时从 git 历史取：`git show 792784d:docs/DESIGN.md`。

## 10. 许可证

本项目采用 [Apache License 2.0](LICENSE)。
上游 BitKindle 的代码与文档按 MIT License 使用，版权归其贡献者所有；归属与 MIT 许可文本见 [NOTICE](NOTICE)。
