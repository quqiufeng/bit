# GCC ladder pipeline

Driver for:

```text
TCC -> GCC 4.7 (two passes) -> GCC 4.8 -> ... -> GCC 14.2
```

The TCC input is the normal C1-built `build/tcc/bit-tcc` seed.  C1 -> TCC
self-hosting is covered by the TCC pipeline; this driver starts from TCC.

## Status

A clean run of this driver completed:

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

Reproduction:

```sh
cd /home/quqiufeng/bit
JOBS=4 BUILD_BASE=/opt/gcc-ladder-final sh tests/gcc-ladder/run.sh
```

The completed run used:

```text
build: /opt/gcc-ladder-final
log:   /tmp/gcc-ladder-final.log
```

Each stage builds and tests:

```text
all-gcc
all-target-libgcc
all-target-libstdc++-v3
C++ hello using the stage's own cc1plus and libstdc++
```

The run is resumable: a stage whose `$BUILD_BASE/log/<stage>-hello.out`
already exists is skipped.  A newly started `BUILD_BASE` always rebuilds
from 4.7.

## Two-pass GCC 4.7

Pass 1 uses only TCC:

```text
TCC compiles gcc/*.c and gcc/cp/*.c
  -> cc1, cc1plus
  -> cc1plus compiles libstdc++
  -> TCC-built GCC 4.7 target libgcc/libstdc++
```

Pass 2 rebuilds GCC 4.7 using the pass-1 build tree:

```text
pass-1 gcc/xgcc -> stage2 GCC 4.7
stage2 GCC 4.7 -> GCC 4.8
```

GCC 4.7 frontends and target libraries come from the self-hosted chain.  GCC 4.7
configure still runs its hard-coded AC_PATH_PROG checks, but the results
are not used for target builds; target CC/CXX come from TCC or the
previous build-tree wrapper.

## Build policy

| Stage | all-gcc C++ mode | libstdc++ mode |
| --- | --- | --- |
| 4.7 pass 1 | TCC (C frontends) | `gnu++11` |
| 4.7 pass 2 | `gnu++98` | `gnu++11` |
| 4.8 | `gnu++98` | `gnu++11` |
| 4.9 | `gnu++11` | `gnu++11` |
| 5.5 | `gnu++11` | `gnu++11` |
| 7.5 | `gnu++11` | `gnu++11` |
| 9.5 | `gnu++11` | `gnu++11` |
| 12.3 | `gnu++11` | `gnu++11` |
| 13.3 | `gnu++11` | `gnu++11` |
| 14.2 | `gnu++11` | `gnu++11` |

GCC 12 libcody checks for exactly C++11, so the ladder uses `gnu++11`.

The driver creates `-B`/`-L` wrappers around each previous stage's build
tree.  The C++ wrapper treats compiles as C++ and links with the previous
stage's libstdc++.  Target libraries are not taken from the host.

## Patches applied by the driver

```text
GCC 4.7 cfns.gperf / cfns.h:
  remove stale libc_name_p prototype

GCC 4.7 onward libgcc/config/i386/linux-unwind.h:
  struct ucontext -> ucontext_t

TCC linking GCC 4.7:
  provide __dso_handle for TCC

TCC building GCC 4.7 soft-fp:
  longlong.h      64-bit add/sub/mul/div/clz fallbacks
  sfp-machine.h   disable fenv inline asm under TCC
  op-common.h     TCC __FP_CLZ fallback
  gcc/system.h    TCC limits/fcntl/stat/dlfcn/mman include fixes

GCC 5.5 i386.h:
  remove const from stringop_strategy members so the 4.9-built
  stage accepts omitted aggregate initializers
```

The soft-fp/header patch is `tests/gcc47/tcc-softfp.patch`.

## Output

```text
$BUILD_BASE/<stage>/            stage build tree
$BUILD_BASE/wrap/               compiler wrappers
$BUILD_BASE/log/<stage>-*.log   logs
$BUILD_BASE/log/<stage>-hello.out
```

A stage passes only after its own C++ hello is compiled and run.

## Not yet covered

```text
Cleanup of GCC configure AC_PATH_PROG probes (non-blocking noise)
Complete make install for every stage
C1 -> TCC integrated into this same driver
Full GCC testsuite per stage
Completely self-owned binutils / glibc / sysroot
Bit-for-bit reproducible build + full DDC matrix
```
