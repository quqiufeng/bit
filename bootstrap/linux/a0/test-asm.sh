#!/bin/sh
set -eu
dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
tmp=${TMPDIR:-/tmp}/a0-asm-test.$$
trap 'rm -f "$tmp" "$tmp.asm" "$tmp.ref" "$tmp.asm.out" "$tmp.rel" "$tmp.asm.rel"' EXIT
cc -std=c99 -O2 "$dir/a0_ref.c" -o "$tmp.ref"
cc -nostdlib -static -no-pie -Wl,--build-id=none "$dir/a0.S" -o "$tmp.asm"
"$tmp.ref" < "$dir/smoke.a0" > "$tmp"
"$tmp.asm" < "$dir/smoke.a0" > "$tmp.asm.out"
cmp "$tmp" "$tmp.asm.out"
"$tmp.ref" < "$dir/rel.a0" > "$tmp.rel"
"$tmp.asm" < "$dir/rel.a0" > "$tmp.asm.rel"
cmp "$tmp.rel" "$tmp.asm.rel"
chmod +x "$tmp.asm.out"
set +e
"$tmp.asm.out"
status=$?
set -e
test "$status" -eq 42
