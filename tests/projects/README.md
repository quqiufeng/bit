# Small C project validation

These scripts validate bit-tcc against real C projects. Sources live under
`/opt`; build outputs live under `build/`.

| Project | Version | Source | Patch | Result |
| --- | --- | --- | --- | --- |
| cJSON | 1.7.18 | `/opt/cJSON-1.7.18` | none | PASS |
| SQLite | 3.45.3 | `/opt/sqlite-amalgamation-3450300` | none | PASS |
| Lua | 5.4.7 | `/opt/lua-5.4.7` | none | PASS |
| BusyBox | 1.36.1 | `/opt/busybox-1.36.1` | `tests/busybox/busybox-1.36.1-tcc.patch` | PASS |
| GCC libiberty/libcpp | 4.7.4 | `/opt/gcc-4.7.4` | none | probes PASS |
| GCC cc1 | 4.7.4 | `/opt/gcc-4.7.4` | none | PASS |
| GCC xgcc | 4.7.4 | `/opt/gcc-4.7.4` | none | PASS |
| GCC C frontend selfhost probe | 4.7.4 | `/opt/gcc-4.7.4` | none | PASS |
| GCC stage2 cc1 | 4.7.4 | `/opt/gcc-4.7.4` | none | PASS |
| GCC libgcc | 4.7.4 | `/opt/gcc-4.7.4` | `tests/gcc47/libgcc-ucontext.patch` | PASS |
| GCC stage2 xgcc | 4.7.4 | `/opt/gcc-4.7.4` | none | PASS |
| GCC stage2/stage3 cc1 | 4.7.4 | `/opt/gcc-4.7.4` | none | PASS |

## Commands

```sh
./tests/cjson/run.sh
./tests/sqlite/run.sh
./tests/lua/run.sh
./tests/busybox/run.sh
./tests/gcc47/run.sh
./tests/gcc47/cc1.sh
./tests/gcc47/xgcc.sh
./tests/gcc47/selfhost.sh
./tests/gcc47/stage2.sh
./tests/gcc47/libgcc.sh
./tests/gcc47/driver.sh
./tests/gcc47/stage3.sh
```

The project scripts skip when their source directory is absent.

## What each script proves

```text
cJSON     single-file library, malloc/stdio/math
SQLite    very large single-file amalgamation, pthread/dl/math link
Lua       multi-file build, liblua.a archive, hosted executable
BusyBox   full defconfig applet build, shell/init userland target
GCC 4.7   libiberty, libcpp, gcc infrastructure, x86 backend and C frontend probes; first GCC bootstrap layers
```

Only BusyBox currently needs source patches. The other three compile directly
with bit-tcc and use bit-tcc for the final executable link.
