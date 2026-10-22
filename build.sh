#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
"$ROOT/build-core.sh"
if [ "${HOSTED:-0}" = 1 ]; then
    "$ROOT/build-hosted.sh"
fi
