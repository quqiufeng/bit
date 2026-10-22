# Linux A1 reference

`a1_ref.c` is the host-side semantics oracle for the Linux A1 layer.  It adds
to A0:

- named labels: `:name`
- named relative references: `%name`
- strings: `"text"`
- zero fill: `.zero N`
- the A0 forms `^`, `:HHHH`, `%HHHH` and raw hex bytes

`smoke.a1` builds a two-PT_LOAD ELF that jumps over a string and exits 42.
`test.sh` assembles and runs it.

`a1.c` is the freestanding body.  It writes `%name`/`%HHHH` as a 4-byte
little-endian PC-relative displacement.  `.zero` accepts a decimal constant
only; it does not evaluate label expressions.

`a1.seed.hex` is reproduced by A0 and by A1 itself.  Source-level A1
self-hosting is still open; the next milestone is C0 for `bootstrap.c`.
