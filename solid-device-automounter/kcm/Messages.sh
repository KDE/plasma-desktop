#! /bin/sh
$EXTRACTRC `find . -name \*.ui -o -name \*.kcfg` >> rc.cpp
$XGETTEXT *.cpp -o $podir/kcm_device_automounter.pot
