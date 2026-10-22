#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)
BUILD=${BUILD:-"$ROOT/build"}
python3 - "$BUILD" "$ROOT/bootstrap" <<'PY'
import binascii, pathlib, sys, textwrap
build = pathlib.Path(sys.argv[1])
out = pathlib.Path(sys.argv[2])
artifacts = {
    'seed-c1.hex': 'c1-stage2',
    'seed-link.hex': 'c1link',
    'seed-startup.hex': 'startup_sysv.o',
    'seed-syscalls.hex': 'syscalls_sysv.o',
}
for name, source in artifacts.items():
    data = (build / source).read_bytes()
    text = binascii.hexlify(data).decode()
    (out / name).write_text('\n'.join(textwrap.wrap(text, 64)) + '\n')
    print(name, len(data))
PY
