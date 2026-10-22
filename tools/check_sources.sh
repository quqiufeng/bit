#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
errors=0
fail() { echo "check_sources: $1" >&2; errors=1; }
if ! grep -Fq 'Derived from tombkeeper/BitKindle' "$ROOT/linux/compiler.c"; then
    fail 'linux/compiler.c is missing its upstream provenance header'
fi
if grep -Eq 'windows\.h|pe_header|runtime_function' "$ROOT/linux/compiler.c"; then
    fail 'linux/compiler.c still contains retired Windows/PE code'
fi
if ! grep -Fq 'MIT' "$ROOT/NOTICE" || ! grep -Fq 'BitKindle' "$ROOT/NOTICE"; then
    fail 'NOTICE is missing the upstream MIT attribution'
fi
commit=$(sed -n 's/.*"commit"[[:space:]]*:[[:space:]]*"\([0-9a-f]*\)".*/\1/p' "$ROOT/windows/upstream.lock" | head -1)
[ ${#commit} -eq 40 ] || fail 'windows/upstream.lock must pin a full 40-character commit'
grep -Fq '"license": "MIT"' "$ROOT/windows/upstream.lock" || fail 'windows/upstream.lock must record the MIT license'
[ -f "$ROOT/windows/README.md" ] || fail 'windows/README.md is missing'
grep -o '"path"[[:space:]]*:[[:space:]]*"[^"]*"' "$ROOT/shared/sources.json" |
    sed 's/.*"\([^"]*\)"$/\1/' |
    while IFS= read -r path; do
        [ -f "$ROOT/$path" ] || { echo "check_sources: missing derived source: $path" >&2; exit 1; }
    done
[ "$errors" -eq 0 ] || exit 1
echo "PASS: source provenance and Linux/PE boundary"
