#!/bin/sh
set -eu
dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
out=${TMPDIR:-/tmp}/a0-selfhost.$$
trap 'rm -f "$out"' EXIT
"$dir/a0.seed.elf" < "$dir/a0.seed.hex" > "$out"
chmod +x "$out"
cmp "$dir/a0.seed.elf" "$out"
