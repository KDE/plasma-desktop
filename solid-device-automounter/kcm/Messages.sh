#! /bin/sh
$EXTRACTRC `find . -name \*.ui -o -name \*.kcfg` >> rc.cpp
$XGETTEXT *.cpp -o $podir/kcm5_device_automounter.pot
