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

Each case compares the process exit status. Some suites also share stdout
checks through the normal test runners.

## Gaps

- The fuzz generator is template based, not grammar/AST based.
- ABI coverage is concentrated in hand-written cases.
- There is no complete ISO C test suite.
- There is no generated test for register exhaustion with mixed
  integer/SSE/aggregate arguments.
- There is no generated test for nested calls, struct returns, or
  long-double stack arguments.

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

## Commands

```sh
BUILD=/tmp/fuzz tests/fuzz-expr.sh
BUILD=/tmp/fuzz tests/fuzz-struct.sh
BUILD=/tmp/fuzz tests/fuzz-float.sh
BUILD=/tmp/fuzz tests/fuzz-varargs.sh
HOSTED=1 ./build.sh
```

The zero-dependency native checks remain:

```sh
./build.sh
tests/clean-room.sh
```
