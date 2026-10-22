#!/bin/sh
set -eu
dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
tmp=$(mktemp -d "${TMPDIR:-/tmp}/linux-seed.XXXXXX")
trap 'rm -rf "$tmp"' EXIT HUP INT TERM
a0="$tmp/a0.seed.elf"
a1="$tmp/a1.seed.elf"
"$dir/seed.elf" < "$dir/seed.hex" > "$tmp/seed.elf"
cmp "$dir/seed.elf" "$tmp/seed.elf"
"$dir/a0/from-hex.sh" "$a0"
"$dir/seed.elf" < "$dir/a0/a0.seed.hex" > "$tmp/a0.from-seed.elf"
cmp "$a0" "$tmp/a0.from-seed.elf"
"$dir/a1/from-hex.sh" "$a1"
"$a0" < "$dir/a1/a1.seed.hex" > "$tmp/a1.from-a0.elf"
cmp "$a1" "$tmp/a1.from-a0.elf"
"$a1" < "$dir/a1/a1.seed.hex" > "$tmp/a1.from-a1.elf"
cmp "$a1" "$tmp/a1.from-a1.elf"
