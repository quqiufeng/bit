#!/bin/sh
# Two-stage TCC-to-GCC pipeline.
#
# Stage 1, PHASE=tcc:
#   seed -> C1 -> TCC self-host -> Lua 5.4.7 -> BusyBox 1.36.1
#
# Stage 2, PHASE=gcc:
#   TCC -> GCC 4.7 pass1/pass2 -> GCC 4.8 ... 14.2
#
# PHASE=ecosystem runs only the TCC ecosystem checks (debug helper).
# PHASE=all runs both stages.
#
# C1 can be reused with C1=...; otherwise the zero-dependency seed chain
# builds it.  BUILD_C1=0 uses build/native-c1/c1.elf without rebuilding.
set -eu

ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
C1=${C1:-}
TCC_OUT=${TCC_OUT:-"$ROOT/build/tcc"}
TCC=${TCC:-"$TCC_OUT/bit-tcc"}
BUILD_BASE=${BUILD_BASE:-/opt/gcc-ladder-final}
JOBS=${JOBS:-2}
PHASE=${PHASE:-all}

need() { [ -x "$1" ] || { echo "missing executable: $1" >&2; exit 1; }; }

build_c1() {
    [ -n "$C1" ] && { need "$C1"; return; }
    if [ "${BUILD_C1:-1}" = 1 ]; then
        ZERO_BUILD=${ZERO_BUILD:-"$ROOT/build/zero"}
        echo "== seed -> C1 =="
        BUILD="$ZERO_BUILD" "$ROOT/build-zero.sh"
        C1="$ZERO_BUILD/native-c1/c1.elf"
    else
        C1="$ROOT/build/native-c1/c1.elf"
    fi
    need "$C1"
}

build_tcc() {
    build_c1
    echo "== C1 -> TCC -> TCC self-host =="
    C1="$C1" OUT="$TCC_OUT" "$ROOT/tests/tcc/upstream.sh"
    need "$TCC"
}

tcc_ecosystem() {
    need "$TCC"
    echo "== TCC -> Lua 5.4.7 =="
    CC="$TCC" "$ROOT/tests/lua/run.sh"
    echo "== TCC -> BusyBox 1.36.1 =="
    CC="$TCC" "$ROOT/tests/busybox/run.sh"
}

gcc_ladder() {
    need "$TCC"
    echo "== TCC -> GCC 4.7 ... 14.2 =="
    TCC="$TCC" BUILD_BASE="$BUILD_BASE" JOBS="$JOBS" \
        "$ROOT/tests/gcc-ladder/run.sh"
}

case "$PHASE" in
    tcc)
        build_tcc
        tcc_ecosystem
        ;;
    ecosystem) tcc_ecosystem ;;
    gcc)       gcc_ladder ;;
    all)
        build_tcc
        tcc_ecosystem
        gcc_ladder
        ;;
    *)
        echo "unknown PHASE: $PHASE (use tcc, ecosystem, gcc, or all)" >&2
        exit 1
        ;;
esac

echo "PASS: phase $PHASE"
