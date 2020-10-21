#!/usr/bin/env bash

find . -regex '.*\.\(cpp\|h\)' -exec cat {} \; | diff -u <(find ./ -regex '.*\.\(cpp\|h\)' -exec clang-format -style=file {} \;) -
