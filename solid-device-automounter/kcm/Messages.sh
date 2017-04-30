#! /bin/sh
$EXTRACTRC *.ui *.kcfg >> rc.cpp
$XGETTEXT *.cpp -o $podir/kcm5_device_automounter.pot
