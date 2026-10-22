# Linux native bootstrap

## Current fixed point

The native path is:

```text
A1 seed
 -> C0v2 seed
 -> native Linux C1
 -> compiler.c / runtime.c / linker.c
 -> native linker seed
 -> stage1 -> stage2
```

`stage1` and `stage2` are byte-identical.  The path is exercised by
`bootstrap/linux/c0/native-c1.sh` and by `./build.sh`.

The native path uses only committed seeds:

- `bootstrap/linux/a1/a1.seed.elf` / `a1.seed.hex`
- `bootstrap/linux/c0/c0v2.a1`
- `bootstrap/linux/c0/native/compiler-runtime.c`
- `bootstrap/linux/c0/native/linker.seed.elf`
- `bootstrap/linux/c0/native/startup.seed.o`
- `bootstrap/linux/c0/native/syscalls.seed.o`

It no longer invokes Windows PE, `pe2elf`, `build/c1link`, C1-built A1, a
host C compiler, or Python.

## C1 `-E` mode

C1 now accepts `-E`:

```sh
c1 -E source.c > source.i
```

The implementation deliberately does not touch include-file state.  Includes
are loaded into the single `pp_input` arena and switched through
`pp_file_cursor[]` / `pp_file_end[]`; the stable output point is the final
`source[]` written by `pp_put` during `preprocess()`.

After `preprocess()`:

- on failure, return 1;
- in `-E` mode, write `source[0..size)` with `source[i] & 255` to stdout and
  return 0;
- otherwise continue with `position = 4096; next(); ...`.

The output is a token stream, not a byte-for-byte replacement for a host
`cpp -E`: comments and original whitespace are not preserved and no `#line`
markers are emitted.

## Remaining seed work

The next reductions are:

1. Generate `compiler-runtime.c` through C1 `-E` instead of host `cc -E`, then
   apply the deterministic function-pointer/typedef transformations.
2. Remove the standalone startup/syscalls object seeds by emitting startup and
   syscall stubs directly into the A1 text.
3. Replace the committed native linker seed with a first linker generated from
   source by the standalone path.
4. Remove the remaining host `gcc`/`ld` and upstream PE bridge from the global
   `build.sh` seed chain.
