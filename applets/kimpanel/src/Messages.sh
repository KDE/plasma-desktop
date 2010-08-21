#! /usr/bin/env bash
$EXTRACTRC *.kcfg >> rc.cpp
$XGETTEXT *.cpp -o $podir/plasma_applet_kimpanel.pot
