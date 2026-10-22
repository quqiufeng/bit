#!/bin/sh
set -eu
src=$1
helpers=$2
out=$3
awk -v hf="$helpers" -v sf="$src" '
BEGIN {
  h = ""
  n = 0
  while ((getline line < hf) > 0) {
    h = h line "\n"
    if (line ~ /^:/) continue
    t = line
    gsub(/[^0-9A-Fa-f]/, "", t)
    n += length(t) / 2
  }
  close(hf)
  i = 0
  marker = 0
  while ((getline line < sf) > 0) {
    i++
    a[i] = line
    if (line == "^00600000") marker = i
  }
  close(sf)
  count = i
  if (!marker) exit 1
  padline = 0
  for (i = 1; i < marker; i++) if (a[i] ~ /^\.zero [0-9]+$/) padline = i
  if (!padline) exit 1
  split(a[padline], z, " ")
  pad = z[2] - n
  if (pad < 0) exit 1
  for (j = 1; j <= count; j++) {
    if (j == padline) {
      printf "%s", h
      print ".zero " pad
      continue
    }
    print a[j]
  }
}' "$src" > "$out"
