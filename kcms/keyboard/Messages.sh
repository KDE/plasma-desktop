#! /usr/bin/env bash
$EXTRACTRC kcm*.ui >> rc.cpp
$XGETTEXT *.cpp -o $podir/kcmkeyboard.pot
rm -f rc.cpp
