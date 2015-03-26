#!/bin/sh

# Invoke the extractrc script on all .ui, .rc, and .kcfg files in the sources.
# The results are stored in a pseudo .cpp file to be picked up by xgettext.
lst=`find . -name \*.rc -o -name \*.ui -o -name \*.kcfg`
if [ -n "$lst" ] ; then
    $EXTRACTRC $lst >> rc.cpp
fi

# Run xgettext to extract strings from all source files.
$XGETTEXT `find . -name \*.cpp -o -name \*.h -o -name \*.qml` -o $podir/kactivities5.pot
