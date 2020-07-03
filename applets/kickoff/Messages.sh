#! /usr/bin/env bash
$XGETTEXT `find . -name \*.qml -o -name \*.js -o -name \*.cpp` -o $podir/plasma_applet_org.kde.plasma.kickoff.pot
