#!/usr/bin/env python3
import json
import sys
from pathlib import Path

root = Path(__file__).resolve().parent.parent
manifest = json.loads((root / "shared/sources.json").read_text())
errors = []

compiler = (root / "linux/compiler.c").read_text()
if "Derived from tombkeeper/BitKindle" not in compiler:
    errors.append("linux/compiler.c is missing its upstream provenance header")
if "windows.h" in compiler or "pe_header" in compiler or "runtime_function" in compiler:
    errors.append("linux/compiler.c still contains retired Windows/PE code")

notice = (root / "NOTICE").read_text()
if "MIT" not in notice or "BitKindle" not in notice:
    errors.append("NOTICE is missing the upstream MIT attribution")

for entry in manifest["derived"]:
    path = root / entry["path"]
    if not path.is_file():
        errors.append(f"missing derived source: {entry['path']}")

if errors:
    for error in errors:
        print(f"check_sources: {error}", file=sys.stderr)
    sys.exit(1)
print("PASS: source provenance and Linux/PE boundary")
