#!/bin/sh
set -eu
dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
tmp=$(mktemp -d "${TMPDIR:-/tmp}/a0-selfhost.XXXXXX")
trap 'rm -rf "$tmp"' EXIT HUP INT TERM
a0="$tmp/a0.seed.elf"
"$dir/from-hex.sh" "$a0"
"$a0" < "$dir/a0.seed.hex" > "$tmp/a0.out"
chmod +x "$tmp/a0.out"
cmp "$a0" "$tmp/a0.out"
