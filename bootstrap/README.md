# Transitional bootstrap seed

This directory contains a **transitional seed bundle**, not the minimal
upstream A0/A1/C0 seed chain:

- `seed-c1.hex` — prebuilt self-hosted C1 compiler
- `seed-link.hex` — prebuilt freestanding `c1link`
- `seed-startup.hex` — SysV `_start` object
- `seed-syscalls.hex` — SysV syscall stubs

`from-seed.sh` decodes them into `build/` and rebuilds stage1/stage2 with
**no gcc and no system ld**, then checks:

- `runtime1.o == runtime2.o`
- `compiler1.o == compiler2.o`
- `seed-stage1 == seed-stage2`
- the rebuilt compiler compiles and runs `examples/hello.c`

Regenerate the bundle after intentional compiler/linker changes with:

```sh
./bootstrap/make-seed.sh
```

## Hashes

```text
9c67da0e5a64911c885778a883f4a911dddc7d2b358b68d5679048878f257155  bootstrap/seed-c1.hex
0d94737820d1e2a18638bc97a944c6ebeb3fba3063656103926f1dafa3f1f8df  bootstrap/seed-link.hex
1b778c4c307ad0552379d55f0a592ad50013d6cbb6c14f55236e940ddab23194  bootstrap/seed-startup.hex
fe4219b0a528b9da7f583aa5569e38ab46f1cd7fbd5c572a89f8aecb78754023  bootstrap/seed-syscalls.hex
```

The bundle is architecture-specific x86-64 Linux. Replacing it with a minimal
seed → A0/A1 → C0 → C1 chain remains the long-term M5 goal.
