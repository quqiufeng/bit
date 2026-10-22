#!/bin/sh
set -eu
dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
a0="$dir/../a0/a0.seed.elf"
tmp=${TMPDIR:-/tmp}/a1-selfhost.$$
trap 'rm -f "$tmp" "$tmp.a0"' EXIT
"$dir/a1.seed.elf" < "$dir/a1.seed.hex" > "$tmp"
cmp "$dir/a1.seed.elf" "$tmp"
"$a0" < "$dir/a1.seed.hex" > "$tmp.a0"
cmp "$dir/a1.seed.elf" "$tmp.a0"
