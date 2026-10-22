# A0 (zero-based Linux)

A0 is not a full mnemonic assembler.  It is the upstream-style byte/label/address
resolver: it reads a textual hex image, resolves numeric labels and relative
references, and writes an executable image.

## Input language

- `; ...` comment
- hex byte pairs: `48 89 E5`
- `^AAAAAAAA` set the current virtual address (origin), emits nothing
- `:HHHH` set the current output offset, emits nothing
- `%HHHH` emit a 4-byte little-endian PC-relative reference to numeric address
  `HHHH`, relative to the end of the 4-byte field

Numeric labels are the `HHHH` addresses established by `^`/`:`.  A0 does not
encode x86 instructions; the bytes are already in the source.  A1 is the next
layer and adds named labels, strings, zero fill and a small mnemonic set.

## Output

Linux ELF64 ET_EXEC, static, fixed load address.  The ELF header and program
headers are part of the A0 source image, exactly as the upstream PE headers are
part of `a0.a0`.

## Current implementation

- `a0_ref.c`: host C reference, used only as a semantics oracle
- `a0.S`: syscall-only x86-64 body, no libc calls
- `smoke.a0`: two PT_LOAD ELF image, exits 42
- `rel.a0`: forward label reference test

`test.sh` checks the C reference.  `test-asm.sh` compiles `a0.S` with the host
cc/ld, then compares its output byte-for-byte with `a0_ref.c`.

## Constraints

- final A0 must be syscall-only; no gcc, ld, libc or upstream Windows seed
- one read buffer, one output buffer, no external files
- deterministic output
- first milestone: resolve a hand-written A0 image that exits with status 42
- second milestone: assemble A1, then A1 assembles itself twice

`make-seed.sh` uses the host cc/ld only to regenerate `a0.seed.hex` from
`a0.S`; the A0 binary is not committed.  `from-hex.sh [out]` materializes A0
from `seed.elf` and `a0.seed.hex`.  `selfhost.sh` materializes A0, runs it on
its own hex and compares the result byte-for-byte, with no cc/ld involved.

The remaining gap is replacing the initial host-generated `a0.seed.hex` with a
hand-authored minimal seed decoder.
