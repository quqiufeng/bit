#!/bin/sh
set -eu
dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
linux=$(CDPATH= cd -- "$dir/.." && pwd)
tmp=$(mktemp -d "${TMPDIR:-/tmp}/a1-selfhost.XXXXXX")
trap 'rm -rf "$tmp"' EXIT HUP INT TERM
a0="$tmp/a0.seed.elf"
a1="$tmp/a1.seed.elf"
"$linux/a0/from-hex.sh" "$a0"
"$dir/from-hex.sh" "$a1"
"$a1" < "$dir/a1.seed.hex" > "$tmp/a1.out"
chmod +x "$tmp/a1.out"
cmp "$a1" "$tmp/a1.out"
"$a0" < "$dir/a1.seed.hex" > "$tmp/a1.a0.out"
cmp "$a1" "$tmp/a1.a0.out"
