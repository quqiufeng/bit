#!/bin/sh
set -eu
PIE=0
if [ "${1:-}" = "--pie" ]; then
    PIE=1
    shift
fi
if [ "$#" -lt 2 ]; then
    echo "usage: link-hosted.sh [--pie] OUTPUT OBJECT..." >&2
    exit 2
fi
OUTPUT=$1
shift
LD=${LD:-ld}
CRT1=${CRT1:-}
CRTI=${CRTI:-}
CRTN=${CRTN:-}
CRTBEGIN=${CRTBEGIN:-}
CRTEND=${CRTEND:-}
LIBDIR=${LIBDIR:-}
DYNAMIC_LINKER=${DYNAMIC_LINKER:-}
if [ "$PIE" -eq 1 ]; then
    CRT_NAME=Scrt1.o
    PIE_FLAG=-pie
else
    CRT_NAME=crt1.o
    PIE_FLAG=
fi

if [ -z "$CRT1" ]; then
    for directory in /usr/lib/x86_64-linux-gnu /lib/x86_64-linux-gnu /usr/lib64 /lib64 /usr/lib /lib; do
        if [ -f "$directory/$CRT_NAME" ] && [ -f "$directory/crti.o" ] && [ -f "$directory/crtn.o" ]; then
            CRT1=$directory/$CRT_NAME
            CRTI=$directory/crti.o
            CRTN=$directory/crtn.o
            LIBDIR=$directory
            break
        fi
    done
fi
if [ -z "$CRT1" ] && command -v gcc >/dev/null 2>&1; then
    CRT1=$(gcc -print-file-name=$CRT_NAME)
    CRTI=$(gcc -print-file-name=crti.o)
    CRTN=$(gcc -print-file-name=crtn.o)
    LIBDIR=$(dirname "$CRT1")
fi
if [ -z "$CRT1" ] || [ ! -f "$CRT1" ]; then
    echo "link-hosted: cannot locate $CRT_NAME" >&2
    exit 1
fi
if [ -z "$CRTBEGIN" ] && command -v gcc >/dev/null 2>&1; then
    if [ "$PIE" -eq 1 ]; then
        CRTBEGIN=$(gcc -print-file-name=crtbeginS.o)
        CRTEND=$(gcc -print-file-name=crtendS.o)
    else
        CRTBEGIN=$(gcc -print-file-name=crtbegin.o)
        CRTEND=$(gcc -print-file-name=crtend.o)
    fi
fi
if [ -z "$CRTBEGIN" ] || [ ! -f "$CRTBEGIN" ] || [ -z "$CRTEND" ] || [ ! -f "$CRTEND" ]; then
    echo "link-hosted: cannot locate crtbegin/crtend" >&2
    exit 1
fi
if [ -z "$DYNAMIC_LINKER" ]; then
    for linker in /lib64/ld-linux-x86-64.so.2 /lib/x86_64-linux-gnu/ld-linux-x86-64.so.2; do
        if [ -f "$linker" ]; then
            DYNAMIC_LINKER=$linker
            break
        fi
    done
fi
if [ -z "$DYNAMIC_LINKER" ]; then
    echo "link-hosted: cannot locate ld-linux-x86-64.so.2" >&2
    exit 1
fi

exec "$LD" $PIE_FLAG -o "$OUTPUT" -dynamic-linker "$DYNAMIC_LINKER" -z noexecstack \
    "$CRT1" "$CRTI" "$CRTBEGIN" "$@" \
    -L"$LIBDIR" -L/usr/lib/x86_64-linux-gnu -L/lib/x86_64-linux-gnu \
    -lc "$CRTEND" "$CRTN"
