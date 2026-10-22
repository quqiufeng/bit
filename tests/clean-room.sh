#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
TMP=$(mktemp -d "${TMPDIR:-/tmp}/bit-clean.XXXXXX")
trap 'rm -rf "$TMP"' EXIT
git clone --quiet --no-hardlinks "$ROOT" "$TMP/repo"
BUILD="$TMP/build" "$TMP/repo/build-zero.sh"
BUILD="$TMP/build" "$TMP/repo/build-core.sh"
echo "PASS: clean-room native bootstrap from committed seeds"
