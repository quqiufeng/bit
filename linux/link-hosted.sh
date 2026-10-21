#!/bin/sh
set -eu
if [ "$#" -lt 2 ]; then
    echo "usage: link-hosted.sh OUTPUT OBJECT..." >&2
    exit 2
fi
OUTPUT=$1
shift
LD=${LD:-ld}
CRT1=${CRT1:-}
CRTI=${CRTI:-}
CRTN=${CRTN:-}
LIBDIR=${LIBDIR:-}
DYNAMIC_LINKER=${DYNAMIC_LINKER:-}

if [ -z "$CRT1" ]; then
    for directory in /usr/lib/x86_64-linux-gnu /lib/x86_64-linux-gnu /usr/lib64 /lib64 /usr/lib /lib; do
        if [ -f "$directory/crt1.o" ] && [ -f "$directory/crti.o" ] && [ -f "$directory/crtn.o" ]; then
            CRT1=$directory/crt1.o
            CRTI=$directory/crti.o
            CRTN=$directory/crtn.o
            LIBDIR=$directory
            break
        fi
    done
fi
if [ -z "$CRT1" ] && command -v gcc >/dev/null 2>&1; then
    CRT1=$(gcc -print-file-name=crt1.o)
    CRTI=$(gcc -print-file-name=crti.o)
    CRTN=$(gcc -print-file-name=crtn.o)
    LIBDIR=$(dirname "$CRT1")
fi
if [ -z "$CRT1" ] || [ ! -f "$CRT1" ]; then
    echo "link-hosted: cannot locate crt1.o" >&2
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

exec "$LD" -o "$OUTPUT" -dynamic-linker "$DYNAMIC_LINKER" -z noexecstack \
    "$CRT1" "$CRTI" "$@" \
    -L"$LIBDIR" -L/usr/lib/x86_64-linux-gnu -L/lib/x86_64-linux-gnu \
    -lc "$CRTN"
