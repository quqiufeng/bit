#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../../.." && pwd)
BUILD=${BUILD:-"$ROOT/build"}
"$ROOT/bootstrap/linux/c0/c0v2-from-a1-selfhost.sh" >/dev/null
C0="$BUILD/c0v2-from-a1/c0v2.elf" "$ROOT/bootstrap/linux/c0/chain.sh"
