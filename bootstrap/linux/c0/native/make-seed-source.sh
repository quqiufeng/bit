#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../../../.." && pwd)
C1=${C1:-"$ROOT/build/c1-stage2"}
OUT=${1:-"$ROOT/bootstrap/linux/c0/native/compiler-runtime.c"}
TMP=$(mktemp -d "${TMPDIR:-/tmp}/c1-seed.XXXXXX")
trap 'rm -rf "$TMP"' EXIT
(cd "$ROOT" && C1_INCLUDE="$ROOT" "$C1" -E < linux/compiler.c > "$TMP/compiler.i")
(cd "$ROOT" && C1_INCLUDE="$ROOT" "$C1" -E < linux/runtime.c > "$TMP/runtime.i")
cat "$TMP/compiler.i" "$TMP/runtime.i" > "$TMP/all.i"
python3 "$ROOT/bootstrap/linux/c0/native/make-source.py" "$TMP/all.i" "$OUT"
