# Shared layer

The long-term design keeps the C front end, diagnostics, pure algorithms and
language tests independent of the target operating system.  The current Linux
port is still physically concentrated in `linux/compiler.c`, derived from the
upstream BitKindle C1 source; the Windows reference tree is not vendored yet.

This directory records the boundary and the provenance manifest so a future
extraction can be checked instead of silently drifting.

## Rules

- Shared code must not include OS headers, target ABI constants or object-format
  details.
- Linux and Windows fork ABI, runtime, object writer, linker and startup code.
- Upstream-derived files keep their provenance and the MIT notice in `NOTICE`.
- The Linux compiler must not regain the retired PE/Win64 import path.
- `float_parse.c`: floating-token decoding, rounding and x86-64 bit conversion.
- `diagnostic.c`: diagnostic buffer/formatter with a platform write hook.

Run the manifest check with:

```sh
python3 tools/check_sources.py
```
