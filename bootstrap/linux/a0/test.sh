#!/bin/sh
set -eu
dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
tmp=${TMPDIR:-/tmp}/a0-test.$$
trap 'rm -f "$tmp"' EXIT
cc -std=c99 -Wall -Wextra -O2 "$dir/a0_ref.c" -o "$tmp"
"$tmp" < "$dir/smoke.a0" > "$tmp.out"
chmod +x "$tmp.out"
set +e
"$tmp.out"
status=$?
set -e
test "$status" -eq 42
