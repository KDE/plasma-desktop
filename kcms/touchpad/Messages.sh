#!/bin/sh

$EXTRACTRC `find . -path applet -prune -o \( -name \*.rc -o -name \*.ui -o -name \*.kcfg \) -print` >> rc.cpp
$XGETTEXT rc.cpp `find kcm -name \*.qml` `find . -path applet -prune -o \( -name \*.cpp -o -name \*.h \) -print` -o $podir/kcm_touchpad.pot

$XGETTEXT `find applet -name \*.qml -o -name \*.js` -o $podir/plasma_applet_touchpad.pot
