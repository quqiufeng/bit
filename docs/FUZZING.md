# Native C1 testing and fuzzing

## Current state

`tests/sysv` contains 38 hand-written edge-case programs covering:

- SysV register/stack argument passing
- struct/union classification and alignment
- long double, NaN and floating-point conversions
- varargs, packed structs, bitfields
- function pointers, globals, recursion and switch/goto

They are not generated from a syntax tree.

The current fuzz suites are deterministic `awk` templates, not AST generators:

| script | generation | differential |
| --- | --- | --- |
| `tests/fuzz-expr.sh` | 8 seeds of random linear integer expressions | Native C1 + native linker vs GCC |
| `tests/fuzz-struct.sh` | 8 seeds of random struct field order | Native C1 + native linker vs GCC |
| `tests/fuzz-float.sh` | 8 seeds of random long double arithmetic | Native C1 + native linker vs GCC |
| `tests/fuzz-varargs.sh` | 8 seeds of random values to fixed 8-argument varargs | Native C1 + native linker vs GCC |
| `tests/fuzz-abi.sh` | integer arity 0..12, register-to-stack transitions | Native C1 + native linker vs GCC |
| `tests/fuzz-abi-float.sh` | mixed int/double/float parameters | Native C1 + native linker vs GCC |
| `tests/fuzz-abi-struct.sh` | random struct field order, pass and return by value | Native C1 + native linker vs GCC |
| `tests/fuzz-abi-longdouble-stack.sh` | 7 integer args plus long double stack arguments | Native C1 + native linker vs GCC |
| `tests/fuzz-ast.sh` | precedence-aware nested expression ASTs | Native C1 + native linker vs GCC |
| `tests/fuzz-iso.sh` | C89/C99 language subset | Native C1 + native linker vs GCC |
| `tests/fuzz-hosted-abi.sh` | callee-saved pollution, stack alignment, C1/GCC/glibc calls | C1-hosted vs GCC-hosted |
| `tests/fuzz-hosted-longdouble.sh` | long double across glibc math calls | C1-hosted vs GCC-hosted |
| `tests/fuzz-hosted-struct.sh` | struct returns from glibc div/ldiv | C1-hosted vs GCC-hosted |
| `tests/fuzz-hosted-varargs.sh` | printf/snprintf with ints/doubles/long doubles | C1-hosted vs GCC-hosted |

Each case compares the process exit status. Some suites also share stdout
checks through the normal test runners.

## Gaps

- Most generators are template based; `fuzz-ast.sh` now has nested precedence.
- There is still no complete ISO C test suite.
- Generated coverage now includes integer arity 0..12, mixed SSE/integer,
  struct pass/return, and long-double stack arguments.
- Missing: nested calls, deep recursive call frames, and full aggregate
  classification combinations.
- Hosted coverage now includes callee-saved pollution, 16-byte stack
  alignment, long double glibc math, struct returns, and varargs printing.
- Hosted x87 tag-word leakage, >6 stack arguments through the pollution
  trampoline, and full aggregate classification remain open.

## Next priority: ABI passing fuzz

Start with a structured ABI generator before a full expression AST.

Generate functions with:

```text
0..12 arguments
int / unsigned / char / short / long
float / double / long double
small / medium / large structs by value
struct returns
varargs
mixed integer and SSE arguments
```

Generate a caller and callee in the same translation unit, return a checksum
from `main`, and compare Native C1 + native linker against GCC.

Cover especially:

- the 5th, 6th and 7th arguments
- register exhaustion and stack spill
- struct classification and hidden return pointers
- float/double/long double mixed with integer arguments
- char/short sign extension
- varargs after named register arguments

## Second priority: expression AST fuzz

After ABI fuzz, replace the linear expression templates with a small
precedence-aware generator covering:

```text
unary and binary operators
short-circuit && and ||
conditional ?:
casts
array and pointer indexing
function calls
```

## TCC 0.9.27 compatibility

`tests/tcc/upstream.sh` is the real-world integration gate. It uses the
vendored `tcc-0.9.27/` source by default (`TCC_SRC` can override it) and checks:

```text
Native C1 -> tcc.o
tcc.o + c1-init.o -> bit-tcc
bit-tcc -v
bit-tcc -c hello.c
bit-tcc compiles tcc.c
tcc-self1.o == tcc-self2.o
tcc-stage2 runs
```

The script skips cleanly when the TCC source or native C1 is absent. It now
builds `libtcc1.a` with bit-tcc itself, links a native executable, builds a
shared object, and links a program with `-L`/`-l`. The final executable, shared
object and self-hosted compiler are linked by bit-tcc, not gcc; gcc is only used
for the bootstrap `bit-tcc` binary and `c1-init.o`. The minimal BusyBox 1.35 build is covered by
`tests/tcc/busybox.sh`; larger Lua/musl link tests remain future work.

## Commands

```sh
BUILD=/tmp/fuzz tests/fuzz-expr.sh
BUILD=/tmp/fuzz tests/fuzz-struct.sh
BUILD=/tmp/fuzz tests/fuzz-float.sh
BUILD=/tmp/fuzz tests/fuzz-varargs.sh
HOSTED=1 ./build.sh
tests/tcc/upstream.sh
BUSYBOX_SRC=/tmp/busybox-1.35.0 tests/tcc/busybox.sh
```

The zero-dependency native checks remain:

```sh
./build.sh
tests/clean-room.sh
```
