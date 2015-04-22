#! /usr/bin/env bash
$EXTRACTRC `find -name \*.ui` >> rc.cpp || exit 11
$XGETTEXT *.cpp */*.cpp -o $podir/kcmmousetheme.pot
rm -f rc.cpp
