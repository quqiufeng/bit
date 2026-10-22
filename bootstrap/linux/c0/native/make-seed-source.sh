#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../../../.." && pwd)
OUT=${1:-"$ROOT/bootstrap/linux/c0/native/compiler-runtime.c"}
TMP=$(mktemp -d "${TMPDIR:-/tmp}/c1-seed.XXXXXX")
trap 'rm -rf "$TMP"' EXIT

NATIVE=${NATIVE:-}
if [ -z "$NATIVE" ]; then
    for candidate in "${BUILD:-}/native-c1" "$ROOT/build/native-c1" \
                     "$ROOT/build/zero/native-c1" "$ROOT/build/core/native-c1" \
                     "$ROOT/build/native-zero/native-c1"; do
        if [ -x "$candidate/c1.elf" ] && [ -x "$candidate/native-linker" ] &&
           [ -f "$candidate/runtime.o" ]; then
            NATIVE=$candidate
            break
        fi
    done
fi

if [ -n "${C1:-}" ]; then
    :
elif [ -n "$NATIVE" ]; then
    C1="$NATIVE/c1.elf"
else
    C1="$ROOT/build/c1-stage2"
fi
if [ ! -x "$C1" ]; then
    echo "make-seed-source: C1 compiler not found: $C1" >&2
    exit 1
fi

TOOL_SOURCE="$ROOT/bootstrap/linux/c0/native/make-source.c"
TOOL="$TMP/make-source"
if [ -n "$NATIVE" ]; then
    C1_INCLUDE="$ROOT" "$C1" < "$TOOL_SOURCE" > "$TMP/make-source.o"
    "$NATIVE/native-linker" --bootstrap "$TOOL" _start \
        "$NATIVE/runtime.o" "$TMP/make-source.o" >/dev/null
elif [ -x "$ROOT/build/c1link" ] && [ -f "$ROOT/build/startup_sysv.o" ] &&
     [ -f "$ROOT/build/syscalls_sysv.o" ]; then
    C1_INCLUDE="$ROOT" "$C1" < "$TOOL_SOURCE" > "$TMP/make-source.o"
    C1_INCLUDE="$ROOT" "$C1" < "$ROOT/linux/runtime.c" > "$TMP/runtime.o"
    "$ROOT/build/c1link" "$TOOL" _start "$ROOT/build/startup_sysv.o" \
        "$TMP/runtime.o" "$TMP/make-source.o" "$ROOT/build/syscalls_sysv.o"
else
    CC=${CC:-cc}
    if ! command -v "$CC" >/dev/null 2>&1; then
        echo "make-seed-source: no native linker or host C compiler" >&2
        exit 1
    fi
    "$CC" -O2 -DMAKE_SOURCE_HOST -o "$TOOL" "$TOOL_SOURCE"
fi

(cd "$ROOT" && C1_INCLUDE="$ROOT" "$C1" -E < linux/compiler.c > "$TMP/compiler.i")
(cd "$ROOT" && C1_INCLUDE="$ROOT" "$C1" -E < linux/runtime.c > "$TMP/runtime.i")
cat "$TMP/compiler.i" "$TMP/runtime.i" > "$TMP/all.i"
"$TOOL" "$TMP/all.i" > "$TMP/compiler-runtime.c"
cp "$TMP/compiler-runtime.c" "$OUT"
