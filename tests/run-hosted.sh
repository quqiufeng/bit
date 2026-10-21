#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)
BUILD=${BUILD:-"$ROOT/build"}
for mode in exec pie; do
    for source in "$ROOT"/tests/hosted/*.c; do
        name=$(basename "$source" .c)
        "$BUILD/c1-stage2" < "$source" > "$BUILD/hosted-$mode-$name.o"
        if [ "$mode" = pie ]; then
            "$ROOT/linux/link-hosted.sh" --pie "$BUILD/hosted-$mode-$name" "$BUILD/hosted-$mode-$name.o"
        else
            "$ROOT/linux/link-hosted.sh" "$BUILD/hosted-$mode-$name" "$BUILD/hosted-$mode-$name.o"
        fi
        "$BUILD/hosted-$mode-$name"
        echo "PASS: hosted/$mode/$name"
    done
done
