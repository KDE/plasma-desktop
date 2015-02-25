#!/bin/sh

$EXTRACTRC `find src -path src/applet -prune -o \( -name \*.rc -o -name \*.ui -o -name \*.kcfg \) -print` >> rc.cpp
$XGETTEXT rc.cpp `find src -path src/applet -prune -o \( -name \*.cpp -o -name \*.h \) -print` -o $podir/kcm_touchpad.pot

$XGETTEXT `find src/applet -name \*.qml -o -name \*.js` -o $podir/plasma_applet_touchpad.pot
