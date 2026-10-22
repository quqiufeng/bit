#!/bin/sh
set -eu
if [ "$#" -lt 2 ]; then
    echo "usage: link-shared.sh OUTPUT OBJECT..." >&2
    exit 2
fi
OUTPUT=$1
shift
exec "${LD:-ld}" -shared -z noexecstack -o "$OUTPUT" "$@"
