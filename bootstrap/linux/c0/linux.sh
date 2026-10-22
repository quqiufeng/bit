#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../../.." && pwd)
BUILD=${BUILD:-"$ROOT/build"}
"$ROOT/bootstrap/linux/c0/chain.sh"
C1_STAGE1="$BUILD/c0/chain/c1-stage1.elf" "$ROOT/bootstrap/bridge.sh"
echo "PASS: C0 -> Windows C1 -> SysV Linux C1"
