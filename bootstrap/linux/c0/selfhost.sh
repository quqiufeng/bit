#!/bin/sh
set -eu
dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
a0="$dir/../a0/a0.seed.elf"
a1="$dir/../a1/a1.seed.elf"
tmp=${TMPDIR:-/tmp}/c0-selfhost.$$
trap 'rm -f "$tmp" "$tmp.a0" "$tmp.a1"' EXIT
"$a0" < "$dir/c0.seed.hex" > "$tmp.a0"
cmp "$dir/c0.seed.elf" "$tmp.a0"
"$a1" < "$dir/c0.seed.hex" > "$tmp.a1"
cmp "$dir/c0.seed.elf" "$tmp.a1"
