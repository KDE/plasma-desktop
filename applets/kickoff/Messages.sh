#! /usr/bin/env bash
$EXTRACTRC `find . -name \*.rc -o -name \*.ui -o -name \*.kcfg` >> rc.cpp
$XGETTEXT `find . -name \*.qml -o -name \*.js -o -name \*.cpp` -o $podir/plasma_applet_org.kde.plasma.kickoff.pot
rm -f rc.cpp
