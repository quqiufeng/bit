# Linux C0

`c0.c` is the freestanding C0 body.  It currently accepts functions, parameters, locals, global `int`/`int[]`,
`if/else`, `while`, `for`, `break/continue`, calls, recursion, arithmetic,
comparison, logical and bitwise expressions.

Global `int` scalars take 4 bytes and `int` arrays take `N * 4` bytes in the
data segment.  Code is padded to `0x80000`; data starts at `0x500000`; the file
is padded to `0x480000`.  C0 compiles `bootstrap.c` to A1 text and A1 assembles it into an ELF, but the
resulting bootstrap executable currently exits 1.  The failure is after source
read, when `next()` returns token 0; lexer/intern debugging is the next step.

## Emitter contract

- A1 `.zero` accepts only a decimal constant.
- C0 owns `generated_bytes`, starting at file offset 0.
- Header + two program headers are 176 bytes (`0xB0`).
- `.zero 80` moves the entry to `0x100`; `:0100` and `:start` follow.
- `.org 524288` moves the file offset to the data segment; A1 accepts decimal only.
- Each emitted byte increments `generated_bytes`.
- Each `%Lxxxx` reference and each 32-bit immediate contributes 4 bytes.
- Strings contribute their bytes plus the NUL.
- The final `.zero` is computed as `0x480000 - generated_bytes`.

The current image uses a 64 KiB RX segment and a separate RW/BSS segment.

## Local layout decision

When function/codegen support is added:

- scalar locals (`int`, `unsigned`, pointer, `char`) use fixed 8-byte slots
- arrays use their real element size, aligned to 8
- the frame size is rounded to 16 bytes
- locals use `[rbp - offset]`; offsets above 127 use `disp32`
- the first six SysV parameters spill to `[rbp - 8]`, `[rbp - 16]`, ...

This avoids compact `char` slot complexity while keeping pointer arithmetic exact.

## Tests

`build.sh` compiles C0, emits A1 text, lets A1 assemble it, and checks
`exit(42)`.  `selfhost.sh` checks that A0 and A1 can reproduce `c0.seed.elf`
from `c0.seed.hex`.
