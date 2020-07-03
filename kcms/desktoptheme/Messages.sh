#! /usr/bin/env bash
$EXTRACTRC `find . -name "*.kcfg"` >> rc.cpp || exit 11
$XGETTEXT `find . -name "*.cpp" -o -name "*.qml"` -o $podir/kcm_desktoptheme.pot
rm -f rc.cpp
