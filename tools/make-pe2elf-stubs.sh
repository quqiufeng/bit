#!/bin/sh
set -eu
dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
tmp=${TMPDIR:-/tmp}/pe2elf-stubs.$$
trap 'rm -f "$tmp.o" "$tmp.bin" "$tmp.nm"' EXIT
cc -c -fno-pie -o "$tmp.o" "$dir/pe2elf_stubs.S"
objcopy -O binary -j .text "$tmp.o" "$tmp.bin"
nm -n "$tmp.o" > "$tmp.nm"
python3 - "$tmp.bin" "$tmp.nm" "$dir/pe2elf_stubs.bin" "$dir/pe2elf_stubs.json" "$dir/pe2elf_stubs.h" <<'PY'
import json
import sys
bin_path, nm_path, out_bin, out_json, out_header = sys.argv[1:]
data = open(bin_path, 'rb').read()
offs = {}
for line in open(nm_path):
    parts = line.split()
    if len(parts) != 3:
        continue
    addr, kind, name = parts
    if kind in 'tTdDbB' and (name.startswith('stub_') or name.startswith('data_')):
        offs[name[5:]] = int(addr, 16)
open(out_bin, 'wb').write(data)
open(out_json, 'w').write(json.dumps(offs, indent=2, sort_keys=True) + '\n')
lines = [
    '#ifndef PE2ELF_STUBS_H',
    '#define PE2ELF_STUBS_H',
    '',
    'static const unsigned char pe2elf_stub_blob[] = {',
]
for i in range(0, len(data), 12):
    chunk = data[i:i + 12]
    lines.append('    ' + ', '.join('0x%02x' % b for b in chunk) + ',')
lines += [
    '};',
    '#define PE2ELF_STUB_BLOB_SIZE %du' % len(data),
    '',
    'struct pe2elf_stub_offset {',
    '    const char *name;',
    '    unsigned int offset;',
    '};',
    '',
    'static const struct pe2elf_stub_offset pe2elf_stub_offsets[] = {',
]
for name in sorted(offs):
    lines.append('    {%s, %du},' % (json.dumps(name), offs[name]))
lines += [
    '};',
    '#define PE2ELF_STUB_OFFSET_COUNT (sizeof(pe2elf_stub_offsets) / sizeof(pe2elf_stub_offsets[0]))',
    '',
    '#endif',
]
open(out_header, 'w').write('\n'.join(lines) + '\n')
PY
