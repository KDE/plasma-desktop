#! /usr/bin/env bash
$EXTRACTRC `find . -name "*.ui"` >> rc.cpp || exit 11
$XGETTEXT `find . -name "*.cpp" -o -name "*.qml"` -o $podir/kcm_lookandfeel.pot
rm -f rc.cpp
