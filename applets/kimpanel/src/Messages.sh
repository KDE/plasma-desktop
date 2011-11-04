#! /usr/bin/env bash
$EXTRACTRC *.ui *.kcfg >> rc.cpp
$XGETTEXT *.cpp *.h -o $podir/plasma_applet_kimpanel.pot
rm -f rc.cpp
