#!/usr/bin/env python3
import fileinput

for line in fileinput.input():
    if line.startswith("loginMode="):
        print ("# DELETE loginMode")
        line = line.replace("default", "emptySession")
        print (line.strip())
