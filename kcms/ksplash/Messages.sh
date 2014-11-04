#! /usr/bin/env bash
$EXTRACTRC `find . -name "*.ui"` >> rc.cpp || exit 11
$XGETTEXT `find . -name "*.cpp"` -o $podir/kcm_splashscreen.pot
rm -f rc.cpp
