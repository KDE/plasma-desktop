#! /usr/bin/env bash
$XGETTEXT `find . -name \*.qml` -o $podir/plasma_shell_org.kde.plasma.desktop.pot
rm -f rc.cpp
