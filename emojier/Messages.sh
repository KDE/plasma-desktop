#! /usr/bin/env bash
$XGETTEXT `find . -name "*.cpp" -o -name "*.qml"` -o $podir/org.kde.plasma.emojier.pot
