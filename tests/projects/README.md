# Small C project validation

These scripts validate bit-tcc against real C projects. Sources live under
`/opt`; build outputs live under `build/`.

| Project | Version | Source | Patch | Result |
| --- | --- | --- | --- | --- |
| cJSON | 1.7.18 | `/opt/cJSON-1.7.18` | none | PASS |
| SQLite | 3.45.3 | `/opt/sqlite-amalgamation-3450300` | none | PASS |
| Lua | 5.4.7 | `/opt/lua-5.4.7` | none | PASS |
| BusyBox | 1.36.1 | `/opt/busybox-1.36.1` | `tests/busybox/busybox-1.36.1-tcc.patch` | PASS |

## Commands

```sh
./tests/cjson/run.sh
./tests/sqlite/run.sh
./tests/lua/run.sh
./tests/busybox/run.sh
```

The project scripts skip when their source directory is absent.

## What each script proves

```text
cJSON     single-file library, malloc/stdio/math
SQLite    very large single-file amalgamation, pthread/dl/math link
Lua       multi-file build, liblua.a archive, hosted executable
BusyBox   full defconfig applet build, shell/init userland target
```

Only BusyBox currently needs source patches. The other three compile directly
with bit-tcc and use bit-tcc for the final executable link.
