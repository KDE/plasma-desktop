#! /usr/bin/env bash
$EXTRACTRC *.ui >> rc.cpp
$XGETTEXT `find . -name \*.cpp -o -name \*.qml` -o $podir/kcmkclock.pot
