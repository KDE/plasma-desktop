#!/bin/sh

$EXTRACTRC `find . -name \*.rc -o -name \*.ui -o -name \*.kcfg` >> rc.cpp
$XGETTEXT rc.cpp `find kcm -name \*.qml` `find . -name \*.cpp -o -name \*.h` -o $podir/kcm_touchpad.pot
