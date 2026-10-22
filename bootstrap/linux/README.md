# Zero-based Linux seed

This is the new zero-based Linux bootstrap root.  It does not use the upstream
Windows seed, gcc, ld, or the transitional seed bundle.

`seed.hex` decodes to a minimal static ELF64 executable that performs
`exit(0)`.  The next stages must be written on top of this seed:

1. A0 assembler (Linux ELF writer, stdin/stdout syscalls)
2. A1 assembler
3. C0 compiler
4. C1 compiler
5. `C1 stage1 == stage2`

This is intentionally a fresh bootstrap tree; the existing `linux/` SysV
compiler remains the current usable compiler, not the zero-based root.
