#!/bin/sh
set -eu
dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
linux=$(CDPATH= cd -- "$dir/.." && pwd)
root=$(CDPATH= cd -- "$linux/../.." && pwd)
out=${1:-"${BUILD:-$root/build}/seed-graph/c0.seed.elf"}
mkdir -p "$(dirname -- "$out")"
tmp=$(mktemp -d "${TMPDIR:-/tmp}/c0-from-hex.XXXXXX")
trap 'rm -rf "$tmp"' EXIT HUP INT TERM
"$linux/a0/from-hex.sh" "$tmp/a0.seed.elf"
"$tmp/a0.seed.elf" < "$dir/c0.seed.hex" > "$out"
chmod +x "$out"
