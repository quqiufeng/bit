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

Sources: only the nine upstream tarballs are kept (`tests/gcc-ladder/SOURCES.md`,
sha256 in `tests/gcc-ladder/SHA256SUMS`).  A stage whose source tree is absent
extracts it from the tarball on demand (`ensure_src`), then applies the patches
below; the extracted trees are therefore derived artifacts and are not kept.

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

The full build tree is heavy: a completed run of all ten stages came to 19 GB
(6.1 GB of intermediate `.o`, 2.3 GB of `.a`, 5.2 GB of unstripped binaries;
the 14.2 `cc1` alone is 324.8 MB).

## Pruned runtime trees

Once the ladder has passed, `prune.sh` reduces the build tree to compilers that
can still compile and link C and C++:

```sh
sh tests/gcc-ladder/prune.sh /opt/gcc-ladder-2026 /opt/gcc-ladder-bin
# optional: STAGES="4.7-pass1:4.7.4 14.2:14.2.0" ... to pick stages
```

```text
$OUT/<stage>/gcc       stripped cc1 cc1plus lto1 lto-dump xgcc collect2
                       lto-wrapper liblto_plugin.so, crt*.o, libgcc*.a
$OUT/<stage>/lib       libgcc_s.so, libstdc++.so
$OUT/<stage>/include   headers, with include/c++/<ver>/ relinked
$OUT/<stage>/bin       <stage>-gcc / <stage>-g++ wrappers
                       (-B<stage>/gcc -I<stage>/include/c++/<ver>
                        -L<stage>/lib, C++ adds -Wl,-rpath -lstdc++)
$OUT/bin               symlinks to every stage wrapper
$OUT/<stage>/smoke-c, smoke-cxx
                       the smoke programs themselves (kept as evidence)
$OUT/.hdr/             header cache materialized from the tarballs
$OUT/MANIFEST.txt      size + sha256 of every kept file
$OUT/log/              per-stage prune logs, build logs, witness, manifest
```

Measured on the completed run: 19 GB -> **1.3 GB** for all ten stages
(58-196 MB each), every stage compiling and running one C and one C++ program
with its own compiler and its own headers/libstdc++.  Using a pruned stage:

```sh
/opt/gcc-ladder-bin/bin/14.2-gcc hello.c -o hello && ./hello
/opt/gcc-ladder-bin/bin/9.5-g++ hello.cc -o hello
```

`PATH` must still contain a host assembler and linker: the pruned tree supplies
the compiler, headers and runtime libraries only, so `as` and `ld` remain
borrowed (see the trust boundary in `docs/boot.md`).  With `env -i` the driver
fails with `collect2: fatal error: cannot find 'ld'`.

Deleting the build trees afterwards leaves only `log/` and `wrap/`:

```sh
rm -rf /opt/gcc-ladder-2026/{4.7-pass1,4.7-pass2,4.8,4.9,5.5,7.5,9.5,12.3,13.3,14.2}
```

Three things the script has to work around:

```text
Headers in a build tree are symlinks into the source tree, which is deleted
  after the run -> materialized from the tarball and std/bits relinked
gcc/liblto_plugin.so is itself a symlink for 4.7-9.5 -> copied dereferenced,
  else xgcc fails with "-fuse-linker-plugin, but liblto_plugin.so not found"
libstdc++.so lives outside lib/ -> the smoke wrappers pass -rpath and -lstdc++
```

## Not yet covered

```text
Cleanup of GCC configure AC_PATH_PROG probes (non-blocking noise)
Complete make install for every stage
C1 -> TCC integrated into this same driver
Full GCC testsuite per stage
Completely self-owned binutils / glibc / sysroot
Bit-for-bit reproducible build + full DDC matrix
```
