#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
BUILD=${BUILD:-"$ROOT/build"}
C1=${C1:-"$BUILD/native-c1/c1.elf"}
LINK=${LINK:-"$BUILD/native-c1/native-linker"}
RT=${RT:-"$BUILD/native-c1/runtime.o"}
if [ ! -x "$C1" ] || [ ! -x "$LINK" ] || [ ! -f "$RT" ]; then
    echo "SKIP: native C1 not built" >&2
    exit 0
fi
TMP=$(mktemp -d "${TMPDIR:-/tmp}/c1-tcc.XXXXXX")
trap 'rm -rf "$TMP"' EXIT
for source in compat-attribute compat-builtin; do
    C1_INCLUDE="$ROOT" "$C1" < "$ROOT/tests/tcc/$source.c" > "$TMP/$source.o"
    "$LINK" --bootstrap "$TMP/$source" _start "$RT" "$TMP/$source.o" >/dev/null
    "$TMP/$source"
done
echo "PASS: TCC attribute and builtin compatibility probes"
