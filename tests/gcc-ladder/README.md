# GCC ladder pipeline

Experimental driver for:

```text
C1 -> TCC -> GCC 4.7 -> GCC 4.8 -> ... -> GCC 14.2
```

## Status

This pipeline is **not green from TCC**.  What has been verified is the
host-built GCC ladder:

```text
host gcc/g++ -> GCC 4.7 -> GCC 4.8 -> ... -> GCC 14.2
```

The TCC-seeded pipeline currently stops at GCC 4.7 target `libgcc`.  TCC
0.9.27 does not implement `__attribute__((mode(TI)))` or
`__attribute__((mode(TF)))`; GCC 4.7 `libgcc` uses both for 128-bit integer
and 128-bit floating-point soft-fp.  A pipeline run reached:

```text
xgcc: internal compiler error: Segmentation fault (program cc1)
make[1]: *** [libgcc/shared-object.mk:17: addtf3_s.o] Error 4
make[1]: *** [libgcc/shared-object.mk:17: divtf3_s.o] Error 4
```

The current working tree also contains a temporary 4.7 `libgcc` skip.  That
skip is not a successful TCC build and must not be treated as one.

## C and C++ boundary

GCC 4.7 is implemented mainly in C:

```text
gcc/*.c          C
gcc/cp/*.c       C, including the C++ frontend implementation
libstdc++-v3/src C++ runtime sources
```

`cc1plus` is therefore C source and is compiled by `$(CC)` unless GCC is
configured with `--enable-build-with-cxx`.  TCC can in principle compile
`cc1plus`; it cannot compile the C++ sources of `libstdc++`.

The remaining C++ bootstrap is therefore:

```text
libstdc++ C++ sources: host g++
```

not `host g++ compiles gcc/cp/*.c`.

The C parts of GCC 4.7 are intended to be compiled by `bit-tcc`.  System
`as`, `ld`, crt and glibc remain external inputs.  A host-built `libgcc` is
currently a possible bootstrap exception, but it has not yet been wired into
the TCC-seeded path successfully.

## Prerequisites

```text
/home/quqiufeng/bit/build/tcc/bit-tcc
/opt/gcc-4.7.4
/opt/gcc-4.8.5
/opt/gcc-4.9.4
/opt/gcc-5.5.0
/opt/gcc-7.5.0
/opt/gcc-9.5.0
/opt/gcc-12.3.0
/opt/gcc-13.3.0
/opt/gcc-14.2.0
/opt/gcc47-deps          (MPFR/MPC built for GCC)
system gcc/g++, as/ld, glibc
```

## Run

Full ladder:

```sh
./tests/gcc-ladder/run.sh
```

A subset:

```sh
STAGES="4.7 4.8" ./tests/gcc-ladder/run.sh
```

Override locations:

```sh
SRC_BASE=/opt \
BUILD_BASE=/opt/gcc-ladder-build \
PREFIX_BASE=/opt/gcc-ladder \
DEPS=/opt/gcc47-deps \
./tests/gcc-ladder/run.sh
```

Do not use a green 4.8+ result as evidence that the TCC-seeded 4.7 stage
passed.  The 4.8+ stages can all be reached from the host-built 4.7 bridge.

## Per-stage policy

| Stage | GCC source | libstdc++ |
| --- | --- | --- |
| 4.7 | `-std=gnu++98` | `-std=gnu++11` |
| 4.8 | `-std=gnu++98` | `-std=gnu++11` |
| 4.9 | `-std=gnu++98` | `-std=gnu++11` |
| 5.5 | `-std=gnu++98` | `-std=gnu++11` |
| 7.5 | `-std=gnu++11` | `-std=gnu++17` |
| 9.5 | `-std=gnu++14` | `-std=gnu++17` |
| 12.3 | `-std=gnu++11` | `-std=gnu++17` |
| 13.3 | `-std=gnu++11` | `-std=gnu++17` |
| 14.2 | `-std=gnu++11` | `-std=gnu++17` |

GCC 12 libcody requires exactly C++11, not C++14/17.

## Patches applied by the driver

```text
GCC 4.7 cfns.gperf / cfns.h:
  remove stale libc_name_p prototype

GCC 4.7 onward libgcc/config/i386/linux-unwind.h:
  struct ucontext -> ucontext_t

TCC linking GCC 4.7:
  provide __dso_handle for TCC
```

## Output

Each stage installs under:

```text
/opt/gcc-ladder/<version>
```

Logs:

```text
/tmp/gcc-ladder-<version>-*.log
```

A stage passes only when the installed:

```text
bin/gcc
bin/g++
```

compile and run C and C++ hello.

## Not yet covered

```text
TCC -> GCC 4.7 target libgcc
Verified TCC-built cc1 / cc1plus in the ladder
Full GCC testsuite per stage
Completely self-owned binutils / glibc / sysroot
Bit-for-bit reproducible build + full DDC matrix
```

The host-built 4.7 -> 14.2 ladder is real, but it is not proof of a
TCC-origin bootstrap.  The driver must not describe itself as a complete
`C1 -> TCC -> GCC 14` pipeline until the 4.7 `libgcc` problem is resolved or
an explicit host-`libgcc` exception is implemented and verified.
