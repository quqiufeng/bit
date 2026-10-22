#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
TMP=$(mktemp -d "${TMPDIR:-/tmp}/bit-full.XXXXXX")
trap 'rm -rf "$TMP"' EXIT
git clone --quiet --no-hardlinks "$ROOT" "$TMP/repo"
(cd "$TMP/repo" && ./build.sh)
echo "PASS: clean-room full build from the committed repository"
