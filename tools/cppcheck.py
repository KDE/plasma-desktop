#!/usr/bin/env python3

# This script should be executed in build directory
# All command-line arguments are passed to cppcheck
# Example: ../cppcheck.py --enable=all

import json
import os
import shlex
import sys

data = None
with open('compile_commands.json') as database:
    data = json.load(database)

args = [ "cppcheck" ]
files = []

def add_include_dir(include_dir):
    if args.count(include_dir) == 0:
        args.append("-I")
        args.append(include_dir)

with open("CMakeCache.txt") as cache:
    for item in cache:
        item = item.strip()
        if not item or item.startswith("//") or item.startswith("#"):
            continue
        valueStart = item.find("=")
        if valueStart == -1:
            continue
        nameEnd = item.find(":")
        if nameEnd == -1 or nameEnd > valueStart:
            nameEnd = valueStart
        if (item[:nameEnd] == "CMAKE_EXTRA_GENERATOR_C_SYSTEM_INCLUDE_DIRS" or
            item[:nameEnd] == "CMAKE_EXTRA_GENERATOR_CXX_SYSTEM_INCLUDE_DIRS"):
            for include_dir in item[valueStart + 1:].split(";"):
                add_include_dir(include_dir)

for item in data:
    files.append(item["file"])
    for arg in shlex.split(item["command"]):
        if arg.startswith("-D"):
            if args.count(arg) == 0:
                args.append(arg)
        elif arg.startswith("-I"):
            add_include_dir(arg[2:])

args.extend(sys.argv[1:])
args.extend(files)

os.execvp("cppcheck", args)
