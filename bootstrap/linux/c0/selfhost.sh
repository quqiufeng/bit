#!/bin/sh
set -eu
dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
linux=$(CDPATH= cd -- "$dir/.." && pwd)
tmp=$(mktemp -d "${TMPDIR:-/tmp}/c0-selfhost.XXXXXX")
trap 'rm -rf "$tmp"' EXIT HUP INT TERM
a0="$tmp/a0.seed.elf"
a1="$tmp/a1.seed.elf"
"$linux/a0/from-hex.sh" "$a0"
"$linux/a1/from-hex.sh" "$a1"
"$a0" < "$dir/c0.seed.hex" > "$tmp/c0.a0.elf"
"$a1" < "$dir/c0.seed.hex" > "$tmp/c0.a1.elf"
cmp "$tmp/c0.a0.elf" "$tmp/c0.a1.elf"
