#!/bin/sh
set -eu
dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
linux=$(CDPATH= cd -- "$dir/.." && pwd)
root=$(CDPATH= cd -- "$linux/../.." && pwd)
out=${1:-"${BUILD:-$root/build}/seed-graph/a0.seed.elf"}
mkdir -p "$(dirname -- "$out")"
"$linux/seed.elf" < "$dir/a0.seed.hex" > "$out"
chmod +x "$out"
