#! /usr/bin/env bash
$EXTRACTRC `find . -name \*.rc -o -name \*.ui -o -name \*.kcfg` >> rc.cpp
pofile=$podir/plasma_applet_org.kde.desktopcontainment.pot
$XGETTEXT `find . -name \*.qml` -L Java -o $pofile
$XGETTEXT `find . -name \*.cpp` -j -o $pofile
rm -f rc.cpp
