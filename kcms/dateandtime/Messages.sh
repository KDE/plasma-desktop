#! /usr/bin/env bash
$EXTRACTRC `find -name \*.ui` >> rc.cpp
$XGETTEXT `find . -name "*.cpp" -o -name "*.qml"` -o $podir/kcm_clock.pot
