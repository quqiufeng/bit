#!/bin/sh
set -eu
dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
tmp=${TMPDIR:-/tmp}/pe2elf-stubs.$$
trap 'rm -f "$tmp.o" "$tmp.bin" "$tmp.nm"' EXIT
cc -c -fno-pie -o "$tmp.o" "$dir/pe2elf_stubs.S"
objcopy -O binary -j .text "$tmp.o" "$tmp.bin"
nm -n "$tmp.o" > "$tmp.nm"
python3 - "$tmp.bin" "$tmp.nm" "$dir/pe2elf_stubs.bin" "$dir/pe2elf_stubs.json" <<'PY'
import json
import sys
bin_path, nm_path, out_bin, out_json = sys.argv[1:]
data = open(bin_path, 'rb').read()
offs = {}
for line in open(nm_path):
    parts = line.split()
    if len(parts) != 3:
        continue
    addr, kind, name = parts
    if kind in 'tT' and name.startswith('stub_'):
        offs[name[5:]] = int(addr, 16)
open(out_bin, 'wb').write(data)
open(out_json, 'w').write(json.dumps(offs, indent=2, sort_keys=True) + '\n')
PY
