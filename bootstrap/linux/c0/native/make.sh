#!/bin/sh
set -eu
src=$1
helpers=$2
out=$3
awk -v hf="$helpers" -v sf="$src" '
BEGIN {
  while ((getline line < hf) > 0) h = h line "\n"
  close(hf)
  while ((getline line < sf) > 0) { n++; a[n] = line }
  close(sf)
  marker = 0
  for (i = 1; i <= n; i++) if (a[i] == "^00600000") { marker = i; break }
  if (!marker) exit 1
  padline = 0
  for (i = 1; i < marker; i++) if (a[i] ~ /^\.zero [0-9]+$/) padline = i
  if (!padline) exit 1
  split(a[padline], z, " ")
  pad = z[2] - 523
  if (pad < 0) exit 1
  started = 0
  for (i = 1; i <= n; i++) {
    if (!started && a[i] == ":start") {
      print ":start"
      print "48 8B 3C 24 "
      print "48 8D 74 24 08 "
      print "48 8D 54 FE 08 "
      print "E8 %__c1_start "
      print "89 C7 "
      print "B8 3C 00 00 00 "
      print "0F 05"
      i += 4
      started = 1
      continue
    }
    if (i == padline) {
      printf "%s", h
      print ".zero " pad
      continue
    }
    print a[i]
  }
}' "$src" > "$out"
