#! /usr/bin/env bash
$EXTRACTRC *.ui >> rc.cpp
$XGETTEXT `find . -name "*.cpp" -o -name "*.qml"` -o $podir/kcm_colors.pot
rm -f rc.cpp
