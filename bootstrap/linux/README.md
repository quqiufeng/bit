# Zero-based Linux seed

This is the new zero-based Linux bootstrap root.  It does not use the upstream
Windows seed, gcc, ld, or the transitional seed bundle.

`seed.S` is a hand-written hexadecimal decoder.  `seed.hex` is its own raw
ELF hex; `seed.elf` decodes it back to itself.  `seed.elf` also decodes
`a0/a0.seed.hex` and reproduces `a0/a0.seed.elf`.

The next stages are:

1. Linux A1 assembler
2. Linux C0 compiler
3. Linux C1 from seed
4. `C1 stage1 == stage2`

This is intentionally a fresh bootstrap tree; the existing `linux/` SysV
compiler remains the current usable compiler, not the zero-based root.
