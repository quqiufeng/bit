# Linux C0

`c0.c` is the freestanding C0 body.  It currently accepts:

```c
int main(void) { return N; }
```

and emits A1 text for the A1 assembler.

## Emitter contract

- A1 `.zero` accepts only a decimal constant.
- C0 owns `generated_bytes`, starting at file offset 0.
- Header + two program headers are 176 bytes (`0xB0`).
- `.zero 80` moves the entry to `0x100`; `:0100` and `:start` follow.
- Each emitted byte increments `generated_bytes`.
- Each `%Lxxxx` reference and each 32-bit immediate contributes 4 bytes.
- Strings contribute their bytes plus the NUL.
- The final `.zero` is computed as `0x10000 - generated_bytes`.

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
