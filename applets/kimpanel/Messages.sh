#! /usr/bin/env bash
$XGETTEXT `find . -name \*.qml -name \*.js` -o $podir/plasma_applet_org.kde.kimpanel.pot
rm -f rc.cpp

