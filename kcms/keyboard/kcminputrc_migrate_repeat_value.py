#!/usr/bin/env python3
import sys

for line in sys.stdin:
    line = line.rstrip()
    print(line, file=sys.stderr)
    if line.startswith("KeyboardRepeating=1"):
        print("KeyRepeat=nothing")
    elif line.startswith("KeyboardRepeating="):
        print("KeyRepeat=repeat")

print("# DELETE KeyboardRepeating")
