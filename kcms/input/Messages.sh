#! /usr/bin/env bash
$EXTRACTRC `find -name \*.ui` >> rc.cpp || exit 11
$XGETTEXT *.cpp */*.cpp -o $podir/kcminput.pot
rm -f rc.cpp
