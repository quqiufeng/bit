# A0 (zero-based Linux)

A0 is the first assembler above `bootstrap/linux/seed.bin`.  It is written as
Linux x86-64 machine code/assembly with syscall I/O only.  No gcc, ld, libc or
upstream Windows seed is allowed in this tree.

## First milestone

Input: ASCII A0 source on stdin.
Output: Linux ELF64 ET_EXEC on stdout.

Required subset:

- `db`, `dw`, `dd`, `dq` data
- labels and numeric expressions
- `org`/origin handling
- x86-64 instruction emission for the subset used by A1
- local and forward label fixups
- `.text`/`.bss` layout
- ELF64 header + program headers
- `read`/`write`/`exit` syscalls only

## Non-goals for the first cut

- macros
- includes
- relocatable ELF objects
- external symbols
- optimization

The immediate test is a hand-written source that emits an ELF program which
exits with a fixed status.  The next test is assembling A1 and reaching the
same A1 stage1/stage2 fixed point.
