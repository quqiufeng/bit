# GCC ladder pipeline

This is the reproducible driver for:

```text
C1 -> TCC -> GCC 4.7 -> GCC 4.8 -> ... -> GCC 14.2
```

## Boundary

TCC cannot compile C++.  The only host C++ bootstrap is:

```text
GCC 4.7 C++ frontend: host g++ compiles cp/*.c
```

Everything after GCC 4.7 is built by the previous installed GCC:

```text
4.7 -> 4.8 -> 4.9 -> 5.5 -> 7.5 -> 9.5 -> 12.3 -> 13.3 -> 14.2
```

The C parts of GCC 4.7 are compiled by bit-tcc.  System `as`, `ld`, crt and
glibc remain external inputs.

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
Full GCC testsuite per stage
Completely self-owned binutils / glibc / sysroot
Bit-for-bit reproducible build + full DDC matrix
```

The driver is intentionally explicit about these boundaries rather than hiding
the host C++ bootstrap.
