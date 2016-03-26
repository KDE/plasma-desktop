#! /usr/bin/env bash

lst=`find . -name \*.rc -o -name \*.ui -o -name \*.kcfg`
if [ -n "$lst" ] ; then
    $EXTRACTRC $lst >> rc.cpp
fi

$XGETTEXT `find . -name \*.qml -o -name \*.js -o -name \*.h -o -name \*.cpp` -o $podir/plasmaactivitymanager.pot
rm -f rc.cpp
