#! /usr/bin/env bash
$EXTRACTRC kcm*.ui >> rc.cpp
$XGETTEXT `find . -name "*.cpp" -o -name "*.qml"` -o $podir/kcmkeyboard.pot
rm -f rc.cpp
