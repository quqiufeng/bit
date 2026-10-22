#!/usr/bin/env python3
import sys
data = open(sys.argv[1], 'rb').read()
h = data.hex()
for i in range(0, len(h), 64):
    print(h[i:i+64])
