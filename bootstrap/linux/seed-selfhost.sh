#!/bin/sh
set -eu
dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
tmp=${TMPDIR:-/tmp}/linux-seed.$$
trap 'rm -f "$tmp" "$tmp.a0"' EXIT
"$dir/seed.elf" < "$dir/seed.hex" > "$tmp"
cmp "$dir/seed.elf" "$tmp"
"$dir/seed.elf" < "$dir/a0/a0.seed.hex" > "$tmp.a0"
cmp "$dir/a0/a0.seed.elf" "$tmp.a0"
