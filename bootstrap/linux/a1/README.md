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

This is not yet the self-hosted A1 binary.  The next step is to write the same
decoder in A0/A1 form and have the seed produce it.
