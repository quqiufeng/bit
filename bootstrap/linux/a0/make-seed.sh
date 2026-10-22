#!/bin/sh
set -eu
dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
tmp=${TMPDIR:-/tmp}/a0-seed.$$
trap 'rm -f "$tmp"' EXIT
cc -nostdlib -static -no-pie -Wl,--build-id=none "$dir/a0.S" -o "$tmp"
"$dir/emit-hex.py" "$tmp" > "$dir/a0.seed.hex"
cp "$tmp" "$dir/a0.seed.elf"
chmod +x "$dir/a0.seed.elf"
