#!/usr/bin/env python
import sys

for line in sys.stdin:
    line = line.rstrip()
    print(line, file=sys.stderr)
    if line.startswith("KeyboardRepeating=0"):
        print("KeyRepeat=nothing")
    elif line.startswith("KeyboardRepeating="):
        print("KeyRepeat=accent")

print("# DELETE KeyboardRepeating")
